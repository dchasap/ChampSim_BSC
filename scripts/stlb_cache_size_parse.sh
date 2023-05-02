
HOME="/gpfs/home/bsc18/bsc18186/scratch/ChampSim-MultiPageSize"

source ${HOME}/scripts/benchmarks.sh
CONF="_lru_rd_sz _lru_HugeL1C _lru_HugeCache_HugeDTLB"
tag="_stlb_size"
# get base LRU results
for bench in $SIMPOINTS; do
	#echo -n "$bench" > ${bench}${tag}.csv
	echo -n "" > ./results/${bench}${tag}.csv
	for conf in $CONF; do
		echo -n ",stlb_misses${conf},stlb_mpki${conf},ipc${conf}" >> ./results/${bench}${tag}.csv
	done	
	echo "" >> ./results/${bench}${tag}.csv

	#echo -n "$bench" >> ${bench}${tag}.csv
	for conf in $CONF; do
		file=${HOME}/results/${bench}${conf}.txt
		l1d_misses=$(grep "L1D TOTAL"  $file | tr -s ' '  | cut -d ' ' -f 8)
		stlb_misses=$(grep "STLB TOTAL"  $file | tr -s ' '  | cut -d ' ' -f 8)
		stlb_mpki=$(bc -l <<< "($stlb_misses * 1000) / 1000000000")
		ipc=$(grep "IPC" $file | tail -n 1 | cut -d ' ' -f 5)
		echo -n ",$stlb_misses,$stlb_mpki,$ipc" >> ${bench}${tag}.csv
	done
	echo "" >> ./results/${bench}${tag}.csv
done

