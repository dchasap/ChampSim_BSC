#ifndef TX_SIBLING_PF_H
#define TX_SIBLING_PF_H

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

#include "address.h"
#include "cache.h"
#include "env_var.h"
#include "modules.h"
#include "util/to_underlying.h"

/*
 * TX-Sibling PTE Prefetcher (port from ChampSim_old)
 *
 * Predicts sibling PTE cache lines (PTEs on the same page-table page
 * as the currently accessed PTE) using a stride-based local predictor.
 *
 * Requirements:
 *   - The target cache must activate prefetches on LOAD, PREFETCH, TRANSLATION.
 *   - ptw.cc encodes translation_level + VPN into pf_metadata (lower 4 bits = level,
 *
 * Env vars:
 *   PF_SBLG_TABLE_SIZE     - prefetch table entries (default 128)
 *   PF_SBLG_DEGREE         - max candidates per access (default 1)
 *   PF_SBLG_CONF_THRESHOLD - confidence threshold (default 2)
 *   PF_SBLG_ACC_WINDOW     - usefulness window size (default 32)
 *   PF_SBLG_ACC_THRESHOLD  - usefulness threshold percentage (default 20)
 *   PF_MSHR_GATE_PCT       - MSHR gate percentage (default 50)
 */

struct tx_sibling : public champsim::modules::prefetcher {
private:
  struct Entry {
    uint64_t page_tag = 0;
    int16_t last_cl_off = 0;
    int16_t last_delta = 0;
    uint8_t confidence = 0;
    bool valid = false;
    bool delta_established = false;  // Track if we've seen at least one delta
    uint64_t birth_tick = 0;
    uint32_t issued_prefetches = 0;
  };

  struct UsefulnessController {
    uint64_t bits = 0;
    uint32_t pos = 0;
    uint32_t useful_cnt = 0;
    uint32_t filled = 0;
    uint32_t window_sz;
    uint32_t threshold;
    bool enabled = true;

    UsefulnessController() : window_sz(32), threshold(20) {}
    UsefulnessController(uint32_t w, uint32_t t) : window_sz(w), threshold(t) {}

    void push(bool useful)
    {
      const uint32_t idx = pos % window_sz;
      const bool evicted = (bits >> idx) & 1ULL;
      if (evicted)
        useful_cnt--;
      if (useful) {
        bits |= (1ULL << idx);
        useful_cnt++;
      } else {
        bits &= ~(1ULL << idx);
      }
      pos = (pos + 1) % window_sz;
      if (filled < window_sz)
        filled++;
      if (filled >= window_sz / 2)
        enabled = (useful_cnt * 100 / filled >= threshold);
    }

    bool is_enabled() const { return enabled; }
  };

  std::size_t table_size;
  uint8_t conf_threshold;
  uint32_t mshr_gate_pct;
  uint64_t offset_bits;
  uint64_t cls_per_page;
  uint64_t cls_per_page_bits;
  std::vector<Entry> table;
  UsefulnessController uc;
  uint32_t degree;
  uint64_t access_tick = 0;

  uint64_t entry_allocations = 0;
  uint64_t entry_replacements = 0;
  uint64_t entry_replaced_before_issue = 0;
  uint64_t entry_survived_to_issue = 0;
  uint64_t replaced_entry_lifetime_sum = 0;

  uint64_t reason_level0_skipped = 0;
  uint64_t reason_uc_disabled = 0;
  uint64_t reason_predict_entry_invalid = 0;
  uint64_t reason_predict_tag_mismatch = 0;
  uint64_t reason_predict_conf_blocked = 0;
  uint64_t reason_predict_zero_delta = 0;
  uint64_t reason_predict_issued = 0;
  uint64_t reason_issue_mshr_blocked = 0;
  uint64_t reason_issue_enqueue_failed = 0;

  void train_only(uint64_t address)
  {
    access_tick++;

    const uint64_t cl_addr = address >> offset_bits;
    const uint64_t page_tag = cl_addr >> cls_per_page_bits;
    const int16_t cl_off = static_cast<int16_t>(cl_addr & (cls_per_page - 1));
    auto& pred = table[page_tag % table_size];

    if (pred.valid && pred.page_tag == page_tag) {
      const int16_t delta = cl_off - pred.last_cl_off;
      if (!pred.delta_established) {
        pred.last_delta = delta;
        pred.delta_established = true;
        pred.confidence = (delta != 0) ? 1 : 0;
      } else if (delta == pred.last_delta) {
        if (pred.confidence < 3)
          pred.confidence++;
      } else {
        pred.last_delta = delta;
        pred.confidence = (delta != 0) ? 1 : 0;
      }
    } else {
      if (pred.valid) {
        entry_replacements++;
        replaced_entry_lifetime_sum += (access_tick - pred.birth_tick);
        if (pred.issued_prefetches > 0)
          entry_survived_to_issue++;
        else
          entry_replaced_before_issue++;
      }
      pred.page_tag = page_tag;
      pred.last_delta = 0;
      pred.confidence = 0;
      pred.delta_established = false;
      pred.birth_tick = access_tick;
      pred.issued_prefetches = 0;
      entry_allocations++;
    }

    pred.valid = true;
    pred.last_cl_off = cl_off;
  }

public:
  // Header-only: explicit constructor that mirrors old TxSiblingPrefetcher(uint64_t offset_bits)
  explicit tx_sibling(CACHE* cache) : champsim::modules::prefetcher(cache)
  {
    offset_bits = champsim::to_underlying(cache->OFFSET_BITS);

    const uint64_t page_bits = 12;
    cls_per_page = (1ULL << page_bits) >> offset_bits;
    cls_per_page_bits = 0;
    for (uint64_t tmp = cls_per_page >> 1; tmp; tmp >>= 1)
      cls_per_page_bits++;

    if (auto e = champsim::EnvVar<unsigned long long>::get("PF_SBLG_TABLE_SIZE")) {
      std::size_t value = static_cast<std::size_t>(*e);
      table_size = (value >= 1) ? value : 128;
      if (value < 1)
        std::cerr << "PF_SBLG_TABLE_SIZE must be >= 1, using default 128" << std::endl;
    } else {
      table_size = 128;
    }

    if (auto e = champsim::EnvVar<int>::get("PF_SBLG_CONF_THRESHOLD")) {
      int value = *e;
      if (value < 0 || value > 3) {
        std::cerr << "PF_SBLG_CONF_THRESHOLD must be 0-3, using default 2" << std::endl;
        value = 2;
      }
      conf_threshold = static_cast<uint8_t>(value);
    } else {
      conf_threshold = 2;
    }

    if (auto e = champsim::EnvVar<int>::get("PF_MSHR_GATE_PCT")) {
      int value = *e;
      if (value <= 0 || value > 100) {
        std::cerr << "PF_MSHR_GATE_PCT must be 1-100, using default 50" << std::endl;
        value = 50;
      }
      mshr_gate_pct = static_cast<uint32_t>(value);
    } else {
      mshr_gate_pct = 50;
    }

    uint32_t acc_window = 32;
    if (auto e = champsim::EnvVar<int>::get("PF_SBLG_ACC_WINDOW")) {
      int value = *e;
      if (value >= 4 && value <= 64)
        acc_window = static_cast<uint32_t>(value);
      else
        std::cerr << "PF_SBLG_ACC_WINDOW must be 4-64, using default 32" << std::endl;
    }

    uint32_t acc_threshold = 20;
    if (auto e = champsim::EnvVar<int>::get("PF_SBLG_ACC_THRESHOLD")) {
      int value = *e;
      if (value >= 0 && value <= 100)
        acc_threshold = static_cast<uint32_t>(value);
      else
        std::cerr << "PF_SBLG_ACC_THRESHOLD must be 0-100, using default 20" << std::endl;
    }

    uc = UsefulnessController(acc_window, acc_threshold);

    degree = 1;
    if (auto e = champsim::EnvVar<int>::get("PF_SBLG_DEGREE")) {
      int value = *e;
      if (value >= 1 && value <= 8)
        degree = static_cast<uint32_t>(value);
      else
        std::cerr << "PF_SBLG_DEGREE must be 1-8, using default 1" << std::endl;
    }

    table.resize(table_size);

    std::cout << "PF SiblingPrefetcher: table_size=" << table_size
              << " conf_threshold=" << static_cast<int>(conf_threshold)
              << " mshr_gate_pct=" << mshr_gate_pct
              << " cls_per_page=" << cls_per_page
              << " acc_window=" << acc_window
              << " acc_threshold=" << acc_threshold
              << " degree=" << degree << std::endl;
  }

  uint32_t get_mshr_gate_pct() const { return mshr_gate_pct; }
  void notify_useful() { uc.push(true); }
  void notify_useless() { uc.push(false); }
  void notify_mshr_gate_blocked() { reason_issue_mshr_blocked++; }
  void notify_enqueue_failed() { reason_issue_enqueue_failed++; }
  void record_prefetch_issued(uint64_t /*pf_addr*/) { /* tracking already in Entry::issued_prefetches */ }

  // Header-only candidate interface, mirrors child_ideal.
  // translated_vpn is ignored (kept for interface compatibility with child_ideal).
  std::vector<uint64_t> get_prefetch_candidates(uint64_t address, std::size_t /*translation_level*/, uint64_t /*ip*/, uint64_t /*translated_vpn*/)
  {
    access_tick++;

    const uint64_t cl_addr = address >> offset_bits;
    const uint64_t page_tag = cl_addr >> cls_per_page_bits;
    const int16_t cl_off = static_cast<int16_t>(cl_addr & (cls_per_page - 1));
    auto& pred = table[page_tag % table_size];
    std::vector<uint64_t> candidates;

    if (!uc.is_enabled()) {
      reason_uc_disabled++;
    } else if (!pred.valid) {
      reason_predict_entry_invalid++;
    } else if (pred.page_tag != page_tag) {
      reason_predict_tag_mismatch++;
    } else if (pred.confidence < conf_threshold) {
      reason_predict_conf_blocked++;
    } else if (pred.last_delta == 0) {
      reason_predict_zero_delta++;
    } else {
      for (uint32_t k = 1; k <= degree; ++k) {
        const int16_t next_off = static_cast<int16_t>(cl_off + static_cast<int16_t>(k) * pred.last_delta);
        if (next_off < 0 || next_off >= static_cast<int16_t>(cls_per_page))
          break;
        candidates.push_back(((page_tag << cls_per_page_bits) | static_cast<uint64_t>(next_off)) << offset_bits);
      }
      pred.issued_prefetches += static_cast<uint32_t>(candidates.size());
      reason_predict_issued += static_cast<uint64_t>(candidates.size());
    }

    // Train on this access
    if (pred.valid && pred.page_tag == page_tag) {
      const int16_t delta = cl_off - pred.last_cl_off;
      if (!pred.delta_established) {
        pred.last_delta = delta;
        pred.delta_established = true;
        pred.confidence = (delta != 0) ? 1 : 0;
      } else if (delta == pred.last_delta) {
        if (pred.confidence < 3)
          pred.confidence++;
      } else {
        pred.last_delta = delta;
        pred.confidence = (delta != 0) ? 1 : 0;
      }
    } else {
      if (pred.valid) {
        entry_replacements++;
        replaced_entry_lifetime_sum += (access_tick - pred.birth_tick);
        if (pred.issued_prefetches > 0)
          entry_survived_to_issue++;
        else
          entry_replaced_before_issue++;
      }
      pred.page_tag = page_tag;
      pred.last_delta = 0;
      pred.confidence = 0;
      pred.delta_established = false;
      pred.birth_tick = access_tick;
      pred.issued_prefetches = 0;
      entry_allocations++;
    }

    pred.valid = true;
    pred.last_cl_off = cl_off;
    return candidates;
  }

  void observe_access(uint64_t address, std::size_t /*translation_level*/, uint64_t /*ip*/, uint64_t /*translated_vpn*/)
  {
    train_only(address);
  }

  void print_stats() const
  {
    uint64_t live_entries = 0;
    uint64_t live_entries_with_issue = 0;
    for (const auto& entry : table) {
      if (!entry.valid)
        continue;
      live_entries++;
      if (entry.issued_prefetches > 0)
        live_entries_with_issue++;
    }

    const double avg_replaced_lifetime =
        (entry_replacements != 0) ? static_cast<double>(replaced_entry_lifetime_sum) / static_cast<double>(entry_replacements) : 0.0;

    std::cout << "PF ENTRY-LIFETIME "
              << "ALLOC:" << entry_allocations << " "
              << "REPL:" << entry_replacements << " "
              << "REPL_BEFORE_ISSUE:" << entry_replaced_before_issue << " "
              << "REPL_AFTER_ISSUE:" << entry_survived_to_issue << " "
              << "AVG_REPL_LIFETIME_TICKS:" << avg_replaced_lifetime << " "
              << "LIVE:" << live_entries << " "
              << "LIVE_WITH_ISSUE:" << live_entries_with_issue
              << std::endl;

    std::cout << "PF DROP-REASONS "
              << "UC_DISABLED:" << reason_uc_disabled << " "
              << "PRED_INVALID:" << reason_predict_entry_invalid << " "
              << "PRED_TAG_MISMATCH:" << reason_predict_tag_mismatch << " "
              << "PRED_CONF_BLOCKED:" << reason_predict_conf_blocked << " "
              << "PRED_ZERO_DELTA:" << reason_predict_zero_delta << " "
              << "PRED_ISSUED:" << reason_predict_issued << " "
              << "ISSUE_MSHR_BLOCKED:" << reason_issue_mshr_blocked << " "
              << "ISSUE_ENQUEUE_FAILED:" << reason_issue_enqueue_failed
              << std::endl;
  }

  // ChampSim module interface implementations
champsim::modules::pf_meta_t prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, bool /*useful_prefetch*/, access_type type, champsim::modules::pf_meta_t metadata_in);
  champsim::modules::pf_meta_t prefetcher_cache_fill(champsim::address addr, long set, long way, uint8_t prefetch, champsim::address evicted_addr, champsim::modules::pf_meta_t metadata_in);
  void prefetcher_cycle_operate() {}
  void prefetcher_final_stats() { print_stats(); }
};

inline champsim::modules::pf_meta_t tx_sibling::prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, bool /*useful_prefetch*/, access_type type, champsim::modules::pf_meta_t metadata_in)
{
  // Only activate on TRANSLATION-type accesses (PTW probes)
  if (type != access_type::TRANSLATION)
    return metadata_in;

  // Extract translation_level from lower 4 bits of metadata (encoded by ptw.cc under EXPAND_PREFETCH)
  const std::size_t translation_level = static_cast<std::size_t>(metadata_in & 0xF);

  if (cache_hit) {
    observe_access(addr.to<uint64_t>(), translation_level, ip.to<uint64_t>(), /*translated_vpn=*/0);
    return metadata_in;
  }

  // MSHR gate: back off if MSHR is too full, but still train.
  if (intern_->get_mshr_occupancy_ratio() * 100 >= get_mshr_gate_pct()) {
    notify_mshr_gate_blocked();
    observe_access(addr.to<uint64_t>(), translation_level, ip.to<uint64_t>(), /*translated_vpn=*/0);
    return metadata_in;
  }

  auto candidates = get_prefetch_candidates(addr.to<uint64_t>(), translation_level, ip.to<uint64_t>(), /*translated_vpn=*/0);
  for (auto pf_addr : candidates) {
    if (!prefetch_line(champsim::address{pf_addr}, true, metadata_in))
      notify_enqueue_failed();
  }

  return metadata_in;
}

inline champsim::modules::pf_meta_t tx_sibling::prefetcher_cache_fill(champsim::address /*addr*/, long /*set*/, long /*way*/, uint8_t /*prefetch*/,
                                                                 champsim::address /*evicted_addr*/, champsim::modules::pf_meta_t metadata_in)
{
  return metadata_in;
}

#endif
