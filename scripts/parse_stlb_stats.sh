
HOME="/gpfs/home/bsc18/bsc18186/scratch/ChampSim-MultiPageSize"

source ${HOME}/scripts/benchmarks.sh
REPLACEMENT_POLICY="mpsz_lru"
LRU_DISTANCES='2 4 6 8 10'
#LRU_DISTANCES='16 32 64'
# get base LRU results
echo -n "benchmark,total_small_pages,total_large_pages,small_pages_evicted,large_pages_evicted,stlb_mpki,ipc"
for replacement_policy in $REPLACEMENT_POLICY; do
	for lru_dist in $LRU_DISTANCES; do
		echo -n ",${replacement_policy}_${lru_dist}_small_pages_evicted,${replacement_policy}_${lru_dist}_large_pages_evicted,${replacement_policy}_${lru_dist}_stlb_mpki,${replacement_policy}_${lru_dist}_ipc"
	done
done
echo ""

for bench in $BENCHMARKS; do
	file=${HOME}/results/${bench}_lru.txt
	small_pages_evicted=$(grep "Small pages evicted:"  $file | cut -d ' ' -f 4)
	large_pages_evicted=$(grep "Large pages evicted:"  $file | cut -d ' ' -f 4)	
	small_pages=$(grep "Total small pages:"  $file | cut -d ' ' -f 4)
	large_pages=$(grep "Total large pages:"  $file | cut -d ' ' -f 4)
	stlb_misses=$(grep "STLB TOTAL"  $file | tr -s ' ' | cut -d ' ' -f 8)
	stlb_mpki=$(bc -l <<< "($stlb_misses * 1000) / 1000000000")
	ipc=$(grep "IPC" $file | tail -n 1 | cut -d ' ' -f 5)
	echo -n "$bench,$small_pages,$large_pages,$small_pages_evicted,$large_pages_evicted,$stlb_mpki,$ipc"
	# get MPSZ_LRU results
	for replacement_policy in $REPLACEMENT_POLICY; do
		for lru_dist in $LRU_DISTANCES; do
			file=${HOME}/results/${bench}_${replacement_policy}_thrld${lru_dist}.txt
			small_pages_evicted=$(grep "Small pages evicted:"  $file | cut -d ' ' -f 4)
			#grep "Small pages evicted:"  $file | cut -d ' ' -f 4
			large_pages_evicted=$(grep "Large pages evicted:"  $file | cut -d ' ' -f 4)	
			#small_pages=$(grep "Total small pages:"  $file | cut -d ' ' -f 4)
			#large_pages=$(grep "Total large pages:"  $file | cut -d ' ' -f 4)
			stlb_misses=$(grep "STLB TOTAL"  $file | tr -s ' '  | cut -d ' ' -f 8)
			stlb_mpki=$(bc -l <<< "($stlb_misses * 1000) / 1000000000")
			ipc=$(grep "IPC" $file | tail -n 1 | cut -d ' ' -f 5)
			echo -n ",$small_pages_evicted,$large_pages_evicted,$stlb_mpki,$ipc"
		done
	done
	echo ""
done

