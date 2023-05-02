
EXP_CONF=$1
source ${EXP_CONF}

#mkdir -p dump/${EXP_NAME}

for benchsuite in ${BENCHSUITES}; do
	#echo $benchsuite
	for conf in ${EXPERIMENTS}; do 
		#echo $conf
		if ${BUILD_CHAMPSIM}; then
			./scripts/gen_champsim_conf.py sim_conf/champsim_dev_baseline.json
			./config.sh sim_conf/champsim_${conf}.json
			make
		fi
		./scripts/submit_jobs.sh ${benchsuite} champsim_${conf} _${conf}
	done
done

