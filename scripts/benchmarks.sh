
#source ./scripts/spec_cpu_workloads.sh
#source ./scripts/gap_workloads.sh
source ./scripts/qualcom_srv_workloads.sh

TRACES=${QUALCOM_SRV}

for trace in $TRACES; do
	export suffix=.champsimtrace.xz 
	export bench=${trace%$suffix}
	#echo $bench
	SIMPOINTS="$SIMPOINTS $bench"
done

for simpoint in $SIMPOINTS; do
	export suffix=-*
	export bench=${simpoint%$suffix}
	BENCHMARKS="${BENCHMARKS} ${bench}"
done
BENCHMARKS=$(echo "${BENCHMARKS}" | xargs -n1 | sort -u | xargs)

