#!/bin/bash

HOME="/scratch/nas/3/dchasapi/ChampSim_dev"
TRACE_DIR="/scratch/nas/3/dchasapi/champsim_traces"
#TRACE_DIR="/scratch/nas/3/dchasapi/ChampSim_dev/ipc1_public"
#TRACE_DIR="/scratch/nas/3/dchasapi/ChampSim/smt_traces"
TRACE_EXT_DIR="/gpfs/projects/bsc18/romol/BranchPred/ChampSim/page_size_extensions"
RESULTS_DIR=${HOME}/results

BENCHSUITE=$1 
BIN=$2
DESCR_TAG=$3
DUMP_DIR=${HOME}/dump

source ${HOME}/scripts/benchmarks_all.sh

for trace in $TRACES; do

  export suffix=.champsimtrace.xz 
  export bench=${trace%$suffix}

	echo "${bench}${DESCR_TAG}"

echo "#!/bin/bash
#SBATCH -N 1
#SBATCH -o ${DUMP_DIR}/${bench}${DESCR_TAG}_run.out 
#SBATCH -J chmpS_${bench}${DESCR_TAG}_run
#SBATCH -q all 

export PTP_EXTRA_STATS_FILE=${HOME}/dump/${bench}${DESCR_TAG}_access_rate.csv
#export REUSE_DIST_FILENAME_PREFIX="${HOME}/dump/${bench}${DESCR_TAG}_reuse_dist"
export RECALL_DIST_FILENAME_PREFIX="${HOME}/dump/${bench}${DESCR_TAG}_recall_dist"

gdb -batch -ex "r" -ex "bt" -ex "q" --args ./bin/${BIN} --warmup_instructions 50000000 --simulation_instructions 100000000 ${TRACE_DIR}/$trace
#gdb -batch -ex "r" -ex "bt" -ex "q" --args ./bin/${BIN} --warmup_instructions 500 --simulation_instructions 10000 ${TRACE_DIR}/$trace

" >	simr_${bench}_job.run
		sbatch simr_${bench}_job.run
		#chmod +x simt_${bench}_job.run
		#./simt_${bench}_job.run
		rm simr_${bench}_job.run
done

