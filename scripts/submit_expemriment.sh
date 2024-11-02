#!/bin/bash

EXP_CONF=$1
source ${EXP_CONF}

export_confs=""
for benchsuite in ${BENCHSUITES}; do

	echo ${benchsuite}
	index=0
	for conf in ${CONFIGURATION_TAGS}; do

		base_conf=$(echo $conf | sed "s/:{.*}//g")
		echo $conf
		echo ${base_conf}

		if ${BUILD_CHAMPSIM}; then

			echo "./scripts/gen_champsim_conf.py ./sim_conf/champsim_dev_baseline.json ${base_conf}"
			./scripts/gen_champsim_conf.py ./sim_conf/champsim_dev_baseline.json ${base_conf}
			echo "./config.sh sim_conf/champsim_${base_conf}.json"
			./config.sh sim_conf/champsim_${base_conf}.json
			make
		fi

		stop=0
		while [ ${stop} -lt ${#VAR_DECLARATIONS[@]} ]; do

			curr_conf=${conf}
			stop=0	
			exploring="false"
			for conf_key in ${!VAR_DECLARATIONS[@]}; do

				declare -a CONF_VALUES=(${VAR_DECLARATIONS[$conf_key]})
				conf_value=${CONF_VALUES[0]}

				if [[ ${#CONF_VALUES[@]} -gt 1 ]] && [ "${exploring}" == "false" ]; then
					VAR_DECLARATIONS[$conf_key]=${CONF_VALUES[@]:1}
					exploring="true"
				else
					stop=$(( $stop + 1  ))
				fi
				#echo size:${#CONF_VALUES[@]}
				#echo keys:${CONF_VALUES[@]}
				#echo keys\':${VAR_DECLARATIONS[$conf_key]}
				#echo key:$conf_value

				echo "export ${conf_key}=${conf_value}"
				export ${conf_key}=${conf_value}

				curr_conf=$(echo ${curr_conf} | sed "s/{$conf_key}/$conf_value/g")
			done

			echo 	./scripts/submit_jobs.sh ${benchsuite} champsim_${base_conf} _${curr_conf}
			./scripts/submit_jobs.sh ${benchsuite} champsim_${base_conf} _${curr_conf}

			export_confs="${export_confs} ${curr_conf}"
		done

		# restore configurations
		source ${EXP_CONF}
		index=$(( $index + 1  ))
	done
done

