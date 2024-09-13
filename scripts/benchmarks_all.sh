
TRACE_DIR="/scratch/nas/3/dchasapi/champsim_traces"
#TRACE_DIR="/scratch/nas/3/dchasapi/champsim_traces_smt"
#TRACE_DIR="/scratch/nas/3/gvavouli/arvei_champsim/traces/shuhai"
TRACE_EXT_DIR="/scratch/nas/3/dchasapi/champsim_traces_pgsz_ext"

source ./scripts/spec_cpu_workloads.sh
source ./scripts/gap_workloads.sh
source ./scripts/qualcom_srv_workloads.sh
source ./scripts/qualcomm_srv_workloads.sh
source ./scripts/cloudsuite_workloads.sh
source ./scripts/shuhai_workloads.sh
source ./scripts/server_workloads.sh
#source ./scripts/datacenterGz_workloads.sh

if [ "${BENCHSUITE}" == "qualcom_srv" ];then
			TRACES="${QUALCOM_SRV1} ${QUALCOM_SRV2} ${QUALCOM_SRV3}"
elif [ "${BENCHSUITE}" == "qualcom_srv1" ];then
	    TRACES="${QUALCOM_SRV1}"
elif [ "${BENCHSUITE}" == "qualcom_srv2" ];then
			TRACES="${QUALCOM_SRV2}"
elif [ "${BENCHSUITE}" == "qualcom_srv3" ];then
			TRACES="${QUALCOM_SRV3}"
elif [ "${BENCHSUITE}" == "selected_qualcom_srv"  ]; then
			TRACES="${SELECTED_QUALCOM_SRV}"
elif [ "${BENCHSUITE}" == "top10_qualcom_srv"  ]; then
			TRACES="${TOP_QUALCOM_SRV}"
elif [ "${BENCHSUITE}" == "smt_qualcom_srv"  ]; then
			TRACES="${SMT_QUALCOM_SRV}"
elif [ "${BENCHSUITE}" == "qualcom_server" ]; then
			TRACES="${QUALCOM_SRV}"
elif [ "${BENCHSUITE}" == "selected_qualcom_server" ]; then
			TRACES="${SELECTED_IPC_QUALCOM_SERVER}"
elif [ "${BENCHSUITE}" == "qualcomm_srv_ap"  ]; then
			TRACES="${QUALCOMM_SRV_AP}"
			TRACES_DIR="${QUALCOMM_SRV_AP_DIR}"
elif [ "${BENCHSUITE}" == "selected_qualcomm_srv_ap"  ]; then
			TRACES="${SELECTED_QUALCOMM_SRV_AP}"
			TRACES_DIR="${QUALCOMM_SRV_AP_DIR}"
elif [ "${BENCHSUITE}" == "smt_qualcomm_srv_ap"  ]; then
			TRACES="${SMT_QUALCOMM_SRV_AP}"
			TRACES_DIR="${QUALCOMM_SRV_AP_DIR}"
elif [ "${BENCHSUITE}" == "smt_qualcomm_srv_ap_16i"  ]; then
			TRACES="${SMT_QUALCOMM_SRV_AP_16I}"
			TRACES_DIR="${QUALCOMM_SRV_AP_DIR}"
elif [ "${BENCHSUITE}" == "shuhai"  ]; then
			TRACES="${SHUHAI}"
			TRACES_DIR="${SHUHAI_DIR}"
elif [ "${BENCHSUITE}" == "cloudsuite"  ]; then
			TRACES="${CLOUDSUITE}"
elif [ "${BENCHSUITE}" == "gapp" ]; then
			TRACES="${GAPP}"
			TRACES_DIR="${GAPP_DIR}"
elif [ "${BENCHSUITE}" == "selected_gapp" ]; then
			TRACES="${SELECTED_GAPP}"
			TRACES_DIR="${GAPP_DIR}"
elif [ "${BENCHSUITE}" == "spec" ]; then
			TRACES="${SPEC_CPU_2006} ${SPEC_CPU_2017}"
			TRACES_DIR="${SPEC_CPU_DIR}"
elif [ "${BENCHSUITE}" == "spec_mem" ]; then
			TRACES="${SPEC_MEM_2017}"
			TRACES_DIR="${SPEC_CPU_DIR}"
elif [ "${BENCHSUITE}" == "spec_mockingjay" ]; then
			TRACES="${SPEC_CPU_MOCKINGJAY}"
			TRACES_DIR="${SPEC_CPU_DIR}"
elif [ "${BENCHSUITE}" == "datacenterGz" ]; then
			echo "datacenter traces loading..."
			TRACES="${DATACENTER_Gz}"
			echo $TRACES
			echo "--"
			echo ${DATACENTER_Gz}
			TRACES_DIR="${DATACENTER_Gz_DIR}"
elif [ "${BENCHSUITE}" == "server" ]; then
			TRACES="${SERVER_WORKLOADS}"
			TRACES_DIR="${SERVER_WORKLOADS_DIR}"
elif [ "${BENCHSUITE}" == "test" ]; then 
			#TRACES="srv_408.champsimtrace.xz"
			#TRACES="srv_12.champsimtrace.xz"
			#TRACES="smt_srv85_ap_srv73_ap_1024i.champsimtrace.xz"
			#TRACES="smt_srv12_ap_srv99_ap_1024i.champsimtrace.xz"
			TRACES="srv12_ap.champsimtrace.xz"
			TRACES_DIR="${QUALCOMM_SRV_AP_DIR}"
			#TRACES_DIR="${QUALCOMM_SRV_AP_DIR}"
			#TRACES="smt_srv_12_srv_99_1024i.champsimtrace.xz"
			#TRACES="smt_srv_s60_srv_s61_1024i.champsimtrace.xz"
fi

SIMPOINTS=''
for trace in $TRACES; do
	if [ "${BENCHSUITE}" == "datacenterGz" ]; then
		export suffix=.champsimtrace.gz
	else
		export suffix=.champsimtrace.xz 
	fi
	export bench=${trace%$suffix}
	SIMPOINTS="$SIMPOINTS $bench"
done

BENCHMARKS=''
for simpoint in $SIMPOINTS; do
	export suffix=-*
	export bench=${simpoint%$suffix}
	BENCHMARKS="${BENCHMARKS} ${bench}"
done
BENCHMARKS=$(echo "${BENCHMARKS}" | xargs -n1 | sort -u | xargs)

echo ${SIMPOINTS}
echo ${BENCHMARKS}
