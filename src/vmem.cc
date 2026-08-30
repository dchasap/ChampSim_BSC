/*
 *    Copyright 2023 The ChampSim Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "vmem.h"

#include <algorithm>
#include <cassert>
#include <fmt/core.h>

#include "champsim.h"
#include "dram_controller.h"
#include "env_var.h"
#include "util/bits.h"

using namespace champsim::data::data_literals;

VirtualMemory::VirtualMemory(champsim::data::bytes page_table_page_size, std::size_t page_table_levels, champsim::chrono::clock::duration minor_penalty,
                             MEMORY_CONTROLLER& dram_, std::optional<uint64_t> randomization_seed_)
    : randomization_seed(randomization_seed_), dram(dram_), minor_fault_penalty(minor_penalty), pt_levels(page_table_levels),
      pte_page_size(page_table_page_size),
      next_pte_page(
          champsim::dynamic_extent{champsim::data::bits{LOG2_PAGE_SIZE}, champsim::data::bits{champsim::lg2(champsim::data::bytes{pte_page_size}.count())}}, 0)
{
  assert(pte_page_size > 1_kiB);
  assert(champsim::is_power_of_2(pte_page_size.count()));

  champsim::page_number last_vpage{
      champsim::lowest_address_for_size(champsim::data::bytes{PAGE_SIZE + champsim::ipow(pte_page_size.count(), static_cast<unsigned>(pt_levels))})};
  champsim::data::bits required_bits{LOG2_PAGE_SIZE + champsim::lg2(last_vpage.to<uint64_t>())};
  if (required_bits > champsim::address::bits) {
    fmt::print("[VMEM] WARNING: virtual memory configuration would require {} bits of addressing.\n", required_bits); // LCOV_EXCL_LINE
  }
  if (required_bits > champsim::data::bits{champsim::lg2(dram.size().count())}) {
    fmt::print("[VMEM] WARNING: physical memory size is smaller than virtual memory size.\n"); // LCOV_EXCL_LINE
  }
  populate_pages();
  shuffle_pages();
}

VirtualMemory::VirtualMemory(champsim::data::bytes page_table_page_size, std::size_t page_table_levels, champsim::chrono::clock::duration minor_penalty,
                             MEMORY_CONTROLLER& dram_)
    : VirtualMemory(page_table_page_size, page_table_levels, minor_penalty, dram_, {})
{
}

void VirtualMemory::populate_pages()
{
  assert(dram.size() > 1_MiB);

#if defined(ENABLE_MULTIPLE_PAGE_SIZE)
  // Partition physical memory: the low portion serves 4KiB pages, the top portion serves 2MiB frames.
  // The pools are disjoint, so the 2MiB of physical addresses covered by a large page are contiguous
  // and can never collide with another mapping.
  //
  // The large-pool share follows the requested large-page ratios (the same env vars that
  // O3_CPU::classify_page() consults, so construction order does not matter). It is floored at
  // ~1/8 of memory so that file-driven page assignments can still mark pages large when the
  // ratios are 0%, and capped at ~7/8 so small-page-only workloads keep a working small pool.
  const auto total_ppages = static_cast<uint64_t>(((dram.size() - 1_MiB) / PAGE_SIZE).count());
  const uint64_t pages_per_large_frame = LARGE_PAGE_SIZE / PAGE_SIZE;
  const auto instr_ratio = static_cast<unsigned>(std::clamp(champsim::EnvVar<int>::get_or("INSTR_PAGE_SIZE_DIST", 0), 0, 100));
  const auto data_ratio = static_cast<unsigned>(std::clamp(champsim::EnvVar<int>::get_or("DATA_PAGE_SIZE_DIST", 0), 0, 100));
  const unsigned large_pool_percent = std::clamp(std::max(instr_ratio, data_ratio), 13u, 87u);
  const uint64_t num_large_frames = (total_ppages * large_pool_percent / 100) / pages_per_large_frame;
  const uint64_t num_small_pages = total_ppages - num_large_frames * pages_per_large_frame;

  ppage_free_list.resize(num_small_pages);
#else
  ppage_free_list.resize(((dram.size() - 1_MiB) / PAGE_SIZE).count());
#endif
  assert(ppage_free_list.size() != 0);
  champsim::page_number base_address =
      champsim::page_number{champsim::lowest_address_for_size(std::max<champsim::data::mebibytes>(champsim::data::bytes{PAGE_SIZE}, 1_MiB))};
  for (auto it = ppage_free_list.begin(); it != ppage_free_list.end(); it++) {
    *it = base_address;
    base_address++;
  }

#if defined(ENABLE_MULTIPLE_PAGE_SIZE)
  large_frame_free_list.clear();
  allocated_large_frames.clear();
  if (num_large_frames > 0) {
    const uint64_t pool_start_byte =
        champsim::page_number{champsim::lowest_address_for_size(std::max<champsim::data::mebibytes>(champsim::data::bytes{PAGE_SIZE}, 1_MiB))}.to<uint64_t>()
        * PAGE_SIZE;
    // First frame base, rounded up to a LARGE_PAGE_SIZE boundary past the end of the small pool
    const uint64_t first_frame_page =
        ((pool_start_byte + num_small_pages * PAGE_SIZE) + LARGE_PAGE_SIZE - 1) / LARGE_PAGE_SIZE * pages_per_large_frame;
    for (uint64_t f = 0; f < num_large_frames; ++f) {
      champsim::page_number frame_base{first_frame_page + f * pages_per_large_frame};
      if ((frame_base.to<uint64_t>() + pages_per_large_frame) * PAGE_SIZE <=
          static_cast<uint64_t>(dram.size().count())) {
        large_frame_free_list.push_back(frame_base);
      }
    }
  }
#endif
}

void VirtualMemory::shuffle_pages()
{
  if (randomization_seed.has_value()) {
    std::shuffle(ppage_free_list.begin(), ppage_free_list.end(), std::mt19937_64{randomization_seed.value()});
#if defined(ENABLE_MULTIPLE_PAGE_SIZE)
    std::shuffle(large_frame_free_list.begin(), large_frame_free_list.end(), std::mt19937_64{randomization_seed.value()});
#endif
  }
}

champsim::dynamic_extent VirtualMemory::extent(std::size_t level) const
{
  const champsim::data::bits lower{LOG2_PAGE_SIZE + champsim::lg2(pte_page_size.count()) * (level - 1)};
  const auto size = static_cast<std::size_t>(champsim::lg2(pte_page_size.count()));
  return champsim::dynamic_extent{lower, size};
}

champsim::data::bits VirtualMemory::shamt(std::size_t level) const { return extent(level).lower; }

uint64_t VirtualMemory::get_offset(champsim::address vaddr, std::size_t level) const { return champsim::address_slice{extent(level), vaddr}.to<uint64_t>(); }

uint64_t VirtualMemory::get_offset(champsim::page_number vaddr, std::size_t level) const { return get_offset(champsim::address{vaddr}, level); }

champsim::page_number VirtualMemory::ppage_front() const
{
  assert(available_ppages() > 0);
  return ppage_free_list.front();
}

void VirtualMemory::ppage_pop()
{
  ppage_free_list.pop_front();
  if (available_ppages() == 0) {
    fmt::print("[VMEM] WARNING: Out of physical memory, freeing ppages\n");
    populate_pages();
    shuffle_pages();
  }
}

std::size_t VirtualMemory::available_ppages() const { return (ppage_free_list.size()); }

#if defined(ENABLE_MULTIPLE_PAGE_SIZE)
champsim::page_number VirtualMemory::frame_of(champsim::page_number vpage)
{
  const uint64_t pages_per_large_frame = LARGE_PAGE_SIZE / PAGE_SIZE;
  return champsim::page_number{(vpage.to<uint64_t>() / pages_per_large_frame) * pages_per_large_frame};
}

champsim::page_number VirtualMemory::large_ppage_front() const
{
  assert(!large_frame_free_list.empty());
  return large_frame_free_list.front();
}

void VirtualMemory::large_ppage_pop()
{
  allocated_large_frames.insert(large_frame_free_list.front());
  large_frame_free_list.pop_front();
  if (large_frame_free_list.empty()) {
    fmt::print("[VMEM] WARNING: Out of large (2MiB) physical frames, freeing frames\n"); // LCOV_EXCL_LINE
    populate_pages();
    shuffle_pages();
  }
}
#endif

#if defined(ENABLE_MULTIPLE_PAGE_SIZE)
translation_result VirtualMemory::va_to_pa(uint32_t cpu_num, champsim::page_number vaddr, uint32_t page_class)
{
  constexpr uint32_t LARGE_PAGE_CLASS = 2; // matches O3_CPU::classify_page()

  const uint64_t pages_per_large_frame = LARGE_PAGE_SIZE / PAGE_SIZE;
  const bool frame_is_large = allocated_large_frames.count(frame_of(vaddr)) != 0;

  // VirtualMemory is the source of truth for the mapping granularity. Even when a request was mislabeled by upper
  // levels (e.g., a prefetch that never went through O3_CPU::classify_page()), a page inside an allocated 2MiB
  // frame must be translated at 2MiB granularity so that every consumer splices consistent physical addresses.
  if (page_class == LARGE_PAGE_CLASS || frame_is_large) {
    const champsim::page_number region_key{(vaddr.to<uint64_t>() / pages_per_large_frame) * pages_per_large_frame};
    auto [lpage, lfault] = vpage_to_ppage_map.try_emplace({cpu_num, region_key}, large_ppage_front());

    // this vpage doesn't yet have a ppage mapping
    if (lfault) {
      large_ppage_pop();
    }

    if constexpr (champsim::debug_print) {
      fmt::print("[VMEM] {} paddr: {} vpage: {} large_page: {} fault: {}\n", __func__, lpage->second, region_key, true, lfault);
    }

    return {lpage->second, lfault ? minor_fault_penalty : champsim::chrono::clock::duration::zero(), true};
  }

  auto [ppage, fault] = vpage_to_ppage_map.try_emplace({cpu_num, vaddr}, ppage_front());

  // this vpage doesn't yet have a ppage mapping
  if (fault) {
    ppage_pop();
  }

  if constexpr (champsim::debug_print) {
    fmt::print("[VMEM] {} paddr: {} vpage: {} large_page: {} fault: {}\n", __func__, ppage->second, vaddr, false, fault);
  }

  return {ppage->second, fault ? minor_fault_penalty : champsim::chrono::clock::duration::zero(), false};
}
#else
std::pair<champsim::page_number, champsim::chrono::clock::duration> VirtualMemory::va_to_pa(uint32_t cpu_num, champsim::page_number vaddr)
{
  auto [ppage, fault] = vpage_to_ppage_map.try_emplace({cpu_num, champsim::page_number{vaddr}}, ppage_front());

  // this vpage doesn't yet have a ppage mapping
  if (fault) {
    ppage_pop();
  }

  auto penalty = fault ? minor_fault_penalty : champsim::chrono::clock::duration::zero();

  if constexpr (champsim::debug_print) {
    fmt::print("[VMEM] {} paddr: {} vpage: {} fault: {}\n", __func__, ppage->second, champsim::page_number{vaddr}, fault);
  }

  return std::pair{ppage->second, penalty};
}
#endif

std::pair<champsim::address, champsim::chrono::clock::duration> VirtualMemory::get_pte_pa(uint32_t cpu_num, champsim::page_number vaddr, std::size_t level)
{
  champsim::dynamic_extent pte_table_entry_extent{champsim::address::bits, shamt(level + 1)};
  auto [ppage, fault] =
      page_table.try_emplace({cpu_num, level, champsim::address_slice{pte_table_entry_extent, vaddr}}, champsim::splice(active_pte_page, next_pte_page));

  // this PTE doesn't yet have a mapping
  if (fault) {
    next_pte_page++;
    if (champsim::page_offset{next_pte_page} == champsim::page_offset{0}) {
      active_pte_page = ppage_front();
      ppage_pop();
    }
  }

  auto offset = get_offset(vaddr, level);
  champsim::address paddr{
      champsim::splice(ppage->second, champsim::address_slice{champsim::dynamic_extent{champsim::data::bits{champsim::lg2(pte_entry::byte_multiple)},
                                                                                       static_cast<std::size_t>(champsim::lg2(pte_page_size.count()))},
                                                              offset})};
  if constexpr (champsim::debug_print) {
    fmt::print("[VMEM] {} paddr: {} vaddr: {} pt_page_offset: {} translation_level: {} fault: {}\n", __func__, paddr, vaddr, offset, level, fault);
  }

  auto penalty = minor_fault_penalty;
  if (!fault) {
    penalty = champsim::chrono::clock::duration::zero();
  }

  return {paddr, penalty};
}
