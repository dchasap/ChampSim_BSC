
# GENERIC CONFIGURATION
export BENCHSUITE=smt_qualcom_srv
#export BENCHSUITE="test"
#export VERSION=baseline
#export VERSION=perfectSTLB
#export VERSION=xdip
#export VERSION=xdip_l1d-ptp
#export VERSION=xdip_l2c-ptp
#export CHAMPSIM_BIN=champsim_opt_pref_l1i-128-12_${VERSION}
#export CONF_TAG=opt_pref_baseline
#export CONF_TAG=opt_pref_xdip_l1d-dpte_hit
export CONF_TAG=opt_pref_perfect-itlb_dstlb
#export CHAMPSIM_BIN=champsim_${CONF_TAG}_${VERSION}
export CHAMPSIM_BIN=champsim_${CONF_TAG}

export BUILD_CHAMPSIN=false

# xdip settings
export XDIP_INSTR_POS=0
export XDIP_DATA_POS=2
export XDIP_MAX_LRU=6
export PTP_ENABLE_THRESHOLD=1.0

# EXPERIMENT DESCRIPTOR TAG 
#export DESCR_TAG="_opt_pref_l1i-128-12_l1d-64-8_l2c-1024-8_llc-2048-16_${VERSION}"
export DESCR_TAG="_${CONF_TAG}"

