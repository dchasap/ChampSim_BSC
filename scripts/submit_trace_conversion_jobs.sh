#!/bin/bash

HOME=`pwd`
TRACE_DIR="/gpfs/projects/bsc18/romol/BranchPred/ChampSim/traces"
TRACE_EXT_DIR="/gpfs/projects/bsc18/romol/BranchPred/ChampSim/page_size_extensions"

CONTIGUITY=550

source ${HOME}/scripts/benchmarks_all.sh

TRACES="compute_fp_0.champsimtrace.xz"

for trace in $TRACES; do
	export suffix=.champsimtrace.xz 
	export bench=${trace%$suffix}

	echo "#!/bin/bash
#SBATCH -N 1
##SBATCH -n 1
##SBATCH -c 12
##SBATCH -c 48
#SBATCH -o ${HOME}/dump/convert_trace_${bench}_${CONTIGUITY}.out 
#SBATCH -J convert_trace_${bench}
#SBATCH --time=0:05:00 
##SBATCH --constraint=highmem
##SBATCH --qos=bsc_cs
#SBATCH --qos=debug

module load python/2.7.13
module load boost/1.75.0

${HOME}/bin/trace_converter -i ${trace} -o ${bench}.champsimtrace_ext_${CONTIGUITY}.xz -c ${CONTIGUITY} -v -l 1000
" >	simr_${bench}_job.run
	sbatch simr_${bench}_job.run
	#chmod +x simt_${bench}_job.run
	#./simt_${bench}_job.run
	rm simr_${bench}_job.run
done

