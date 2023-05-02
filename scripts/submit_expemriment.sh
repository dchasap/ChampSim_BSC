
EXP_CONF=$1
source $EXP_CONF
if ${BUILD_CHAMPSIN}; then
	./scripts/gen_champsim_conf.py conf/champsim_opt_pref_baseline_config.json
	./config.sh conf/champsim_${CONF_TAG}.json
	make
fi
./scripts/submit_jobs.sh ${BENCHSUITE} ${CHAMPSIM_BIN} ${DESCR_TAG}
