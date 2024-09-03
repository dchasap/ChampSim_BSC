#!/bin/bash

HOME="/scratch/nas/3/dchasapi/ChampSim_dev"
BENCHSUITE='spec' 

source ${HOME}/scripts/benchmarks_all.sh

TRACE_DIR="/scratch/nas/3/dchasapi/champsim_traces/${TRACES_DIR}"
BRANCH_TRACE_DIR="/scratch/nas/3/dchasapi/champsim_branch_traces"
DUMP_DIR=${HOME}/dump

echo $TRACES_DIR
echo $TRACE_DIR

mkdir -p ${DUMP_DIR}
mkdir -p ${BRANCH_TRACE_DIR}


for trace in $TRACES; do

  export suffix=.champsimtrace.xz 
  export bench=${trace%$suffix}

	echo "${bench}${DESCR_TAG}"

echo "#!/bin/bash
#SBATCH -N 1
#SBATCH -o ${DUMP_DIR}/gen_branch_trace_${bench}.out 
#SBATCH -J gen_branch_trace_${bench}
#SBATCH -q all 
##SBATCH -q large
##SBATCH --time=24:00:00

${HOME}/branch_reader/branch_reader -i ${TRACE_DIR}/${trace} -o ${BRANCH_TRACE_DIR}/${bench}.jsonl -j

" >	simr_${bench}_job.run
		sbatch simr_${bench}_job.run
		#chmod +x simt_${bench}_job.run
		#./simt_${bench}_job.run
		rm simr_${bench}_job.run
done

