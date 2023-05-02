
source ./scripts/benchmarks_all.sh

#SIMPOINTS="403.gcc-16B 403.gcc-17B 403.gcc-48B 450.soplex-92B 450.soplex-247B"
#SIMPOINTS="450.soplex-92B 450.soplex-247B"
#BENCHMARKS="403.gcc 450.soplex"
#BENCHMARKS="450.soplex"

if [ 1 -eq 1 ]; then
echo "Parsing page size distribution stats..."
CONTIGUITY=256
for simpoint in $SIMPOINTS; do
	echo ${simpoint}
	./scripts/parse_page_size_dist.sh ./dump/convert_trace_${simpoint}.out > ./stats/${simpoint}_page_size_dist_${CONTIGUITY}.csv
done

echo "Applying simpoint weights..."
for bench in $BENCHMARKS; do
	echo "ls ./stats/${bench}-*B_page_size_dist.csv"
	data_files=$(ls ./stats/${bench}-*B_page_size_dist.csv)
	echo ${data_files}
  python3 ./scripts/apply_weights.py --input-files ${data_files} --weight-file ./weights/${bench}.wghts --output-file ./stats/${bench}_page_size_dist.csv
done
fi

if [ 1 -eq 0 ]; then
CONF="lru"
FINAL_MERGED_FILE="page_size_eviction_rates"
CONF="mpsz_lru__fully_assoc_thrld2 mpsz_lru_fully_assoc_thrld4 mpsz_lru_fully_assoc_thrld6 mpsz_lru_fully_assoc_thrld8"
FINAL_MERGED_FILE="window_size_comparison_fully_assoc"
#CONF="lru lru_HugeL1D lru_HugeLLC lru_HugeDTLB lru_HugeSTLB"
#FINAL_MERGED_FILE="cache_tlb_sensitivity"

echo "Parsing simpoint simulation data..."
for conf in ${CONF}; do
	for simpoint in ${SIMPOINTS}; do
		echo ${simpoint}
		#./scripts/parse_stlb_size_stats.sh ${simpoint}
		./scripts/parse_stlb_size_stats.sh ./results/${simpoint}_${conf}.txt > ./stats/${simpoint}_${conf}.csv
		echo "./stats/${simpoint}_${conf}.csv"
	done
done
echo "done"

echo "Applying simpoint weights..."
for conf in $CONF; do
	echo "Generating ${conf} data"
	for bench in $BENCHMARKS; do
		echo "ls ./stats/${bench}-*B_${conf}.csv"
		data_files=$(ls ./stats/${bench}-*B_${conf}.csv)
		echo $data_files
		python3 ./scripts/apply_weights.py --input-files $data_files --weight-file ./weights/${bench}.wghts --output-file ./stats/${bench}_avg_${conf}.csv
	done
done
echo "done"

echo "Append benchmarks' data into one file..."
for conf in $CONF; do
		echo "Merging ${conf} data"
		benchmarks=''
		files=''
		for bench in $BENCHMARKS; do
			benchmarks="${benchmarks} $bench"
			files="${files} ./stats/${bench}_avg_${conf}.csv"
		done
		python3 ./scripts/append_stats.py --input-files ${files} --row-names ${benchmarks} --output-file=./stats/${conf}.csv
done
echo "done"

echo "Merging data files for ${CONF}..."
files=''
	for conf in $CONF; do
		files="${files} ./stats/${conf}.csv"
	done

python3 ./scripts/merge_stats.py --input-files ${files} --suffices ${CONF} --output-file ./stats/${FINAL_MERGED_FILE}.csv
echo "done"
fi

### prepare reuse distance histogram data
if [ 1 -eq 0 ]; then
	echo "Applying simpoint weights..."
	for bench in ${BENCHMARKS}; do
		data_files=$(ls ./_results/${bench}*_pow.csv)
		python3 ./scripts/apply_weights.py --input-files $data_files --exclude-columns 'reuse_dist' --weight-file ./weights/${bench}.wghts --output-file ./stats/${bench}_rd.csv
	done
fi

