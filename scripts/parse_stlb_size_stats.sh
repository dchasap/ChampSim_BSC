

HOME="/gpfs/home/bsc18/bsc18186/scratch/ChampSim-MultiPageSize"

file=$1

echo "stlb_misses,stlb_mpki,dtlb_misses,dtlb_mpki,l1d_misses,l1d_mpki,llc_misses,llc_mpki,ipc,smpg_miss2evict,lgpg_miss2evict,avg_small_page_lru,avg_large_page_lru,small_page_eviction_rate,large_page_eviction_rate"

stlb_misses=$(grep "STLB TOTAL"  ${file} | tr -s ' '  | cut -d ' ' -f 8)
stlb_mpki=$(bc -l <<< "($stlb_misses * 1000) / 1000000000")

dtlb_misses=$(grep "DTLB TOTAL"  ${file} | tr -s ' '  | cut -d ' ' -f 8)
dtlb_mpki=$(bc -l <<< "($dtlb_misses * 1000) / 1000000000")

l1d_misses=$(grep "L1D TOTAL"  ${file} | tr -s ' '  | cut -d ' ' -f 8)
l1d_mpki=$(bc -l <<< "($l1d_misses * 1000) / 1000000000")

llc_misses=$(grep "LLC TOTAL"  ${file} | tr -s ' '  | cut -d ' ' -f 8)
llc_mpki=$(bc -l <<< "($llc_misses * 1000) / 1000000000")

ipc=$(grep "IPC" ${file} | tail -n 1 | cut -d ' ' -f 5)

smpg_miss2evict=$(grep "SMALL PAGE MISSES" ${file} | cut -d ' ' -f 7)
lgpg_miss2evict=$(grep "LARGE PAGE MISSES" ${file} | cut -d ' ' -f 7)

total_victim_lookups=$(grep "Total victim lookups:" ${file} | cut -d ':' -f 2)  
total_small_pages=$(grep "Total small pages:" ${file} | head -n 1 | cut -d ':' -f 2) 
total_large_pages=$(grep "Total large pages:" ${file} | head -n 1 | cut -d ':' -f 2)   
total_small_pages_evicted=$(grep "Small pages evicted:" ${file} | cut -d ' ' -f 4)  
total_large_pages_evicted=$(grep "Large pages evicted:" ${file} | cut -d ' ' -f 4)
avg_small_page_lru=$(grep "Avg small page lru:" ${file} | cut -d ':' -f 2) 
avg_large_page_lru=$(grep "Avg large page lru:" ${file} | cut -d ':' -f 2) 
avg_large_page_lru=$(bc -l <<< "$avg_large_page_lru / $total_large_pages")
small_page_eviction_rate=$(bc -l <<< "($total_small_pages_evicted * 1000) / $total_small_pages")
large_page_eviction_rate=$(bc -l <<< "($total_large_pages_evicted * 1000) / $total_large_pages")

echo "$stlb_misses,$stlb_mpki,$dtlb_misses,$dtlb_mpki,$l1d_misses,$l1d_mpki,$llc_misses,$llc_mpki,$ipc,${smpg_miss2evict},${lgpg_miss2evict},${avg_small_page_lru},${avg_large_page_lru},${small_page_eviction_rate},${large_page_eviction_rate}" 

