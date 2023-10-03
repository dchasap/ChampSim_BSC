#!/bin/bash

EXP_CONF=$1
source ${EXP_CONF}

#mkdir -p dump/${EXP_NAME}
export_confs=""
for benchsuite in ${BENCHSUITES}; do

	echo ${benchsuite}
	index=0
	for base_conf in ${BASE_CONFIGURATIONS}; do

		echo ${base_conf}
		if ${BUILD_CHAMPSIM}; then

			#./scripts/gen_champsim_conf.py sim_conf/champsim_dev_baseline.json
			echo "./scripts/gen_champsim_conf_2.py sim_conf/champsim_dev_baseline.json ${base_conf}"
			./scripts/gen_champsim_conf_2.py sim_conf/champsim_dev_baseline.json ${base_conf}
			echo "./config.sh sim_conf/champsim_${base_conf}.json"
			./config.sh sim_conf/champsim_${base_conf}.json
			make
		fi

		stop=0
		while [ ${stop} -lt ${#CONFIGURATIONS[@]} ]; do

			#conf=${CONFIGURATION_TAGS[$index]}
			stop=0	
			exploring="false"
			for conf_key in ${!CONFIGURATIONS[@]}; do

				declare -a CONF_VALUES=(${CONFIGURATIONS[$conf_key]})
				conf_value=${CONF_VALUES[0]}

				if [[ ${#CONF_VALUES[@]} -gt 1 ]] && [ "${exploring}" == "false" ]; then
					CONFIGURATIONS[$conf_key]=${CONF_VALUES[@]:1}
					exploring="true"
				else
					stop=$(( $stop + 1  ))
				fi
				#echo size:${#CONF_VALUES[@]}
				#echo keys:${CONF_VALUES[@]}
				#echo keys\':${CONFIGURATIONS[$conf_key]}
				#echo key:$conf_value

				echo "export ${conf_key}=${conf_value}"
				export ${conf_key}=${conf_value}

				#conf=$(echo $conf | sed "s/{$conf_key}/$conf_value/g")
			done
			#echo cp ./bin/champsim_${base_conf} ./bin/champsim_${conf}
			#cp ./bin/champsim_${base_conf} ./bin/champsim_${conf}
			echo 	./scripts/submit_jobs.sh ${benchsuite} champsim_${base_conf} _${base_conf}
			./scripts/submit_jobs.sh ${benchsuite} champsim_${base_conf} _${base_conf}
			export_confs="${export_confs} dev_${base_conf}"
		done

		# restore configurations
		source ${EXP_CONF}
		index=$(( $index + 1  ))
	done
done

echo $export_confs > ./exp_conf/last_conf.txt
