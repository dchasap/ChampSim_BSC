#!/bin/bash

HOME=`pwd`
TRACE_DIR="/gpfs/projects/bsc18/romol/BranchPred/ChampSim/traces"
SMT_TRACE_DIR="/gpfs/projects/bsc18/romol/BranchPred/ChampSim/smt_traces"

BENCHMARKS="
srv_706
srv_575
srv_s10
srv_582
srv_442
srv_s61
srv_495
srv_41
srv_s68
srv_527
"

BENCHMARKS_BASE_HIGH="
srv_207
srv_s60
srv_12
srv_540
srv_537
"

BENCHMARKS_HIGH="
srv_527
srv_s68
srv_s61
srv_582
srv_s10
"

BENCHMARKS_MID="
srv_353
srv_694
srv_343
srv_99
srv_303
"

BENCHMARKS_LOW="
srv_s43
srv_s42
srv_s52
srv_147
srv_413
"

INSTR_BSZ=1024

#BENCHMARKS2=${BENCHMARKS}
for BENCH1 in $BENCHMARKS_BASE_HIGH; do
#	BENCHMARKS2=$(echo $BENCHMARKS2 | tr -d "${BENCH1}")
#	BENCHMARKS2=$(echo $BENCHMARKS2 | cut -d' ' -f2-)
#	echo $BENCHMARKS2
	for BENCH2 in $BENCHMARKS_LOW; do
#BENCH1=$1 
#BENCH2=$2

echo "#!/bin/bash
#SBATCH -N 1
##SBATCH -n 1
##SBATCH -c 12
##SBATCH -c 48
#SBATCH -o ${HOME}/dump/smt_trace_conversion_${BENCH1}_${BENCH2}_run.out 
#SBATCH -J smt_trace_conversion_${BENCH1}_${BENCH2}_run
##SBATCH --time=00:30:00 
##SBATCH --constraint=highmem
#SBATCH --qos=bsc_cs
##SBATCH --qos=debug

module load python/2.7.13
module load boost/1.75.0


${HOME}/bin/smt_trace_generator -b ${INSTR_BSZ} -i ${TRACE_DIR}/${BENCH1}.champsimtrace.xz -t ${TRACE_DIR}/${BENCH2}.champsimtrace.xz -o ${SMT_TRACE_DIR}/smt_${BENCH1}_${BENCH2}_${INSTR_BSZ}i.champsimtrace.xz
" >	simr_${BENCH1}_${BENCH2}_job.run
		sbatch simr_${BENCH1}_${BENCH2}_job.run
		#chmod +x simt_${bench}_job.run
		#./simt_${bench}_job.run
		rm simr_${BENCH1}_${BENCH2}_job.run
	done 
done
