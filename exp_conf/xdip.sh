
# GENERIC CONFIGURATION
export BENCHSUITE=smt_qualcom_srv
#export BENCHSUITE="test"
#export VERSION=baseline
export VERSION=perfectiSTLB
#export VERSION=xdip
#export VERSION=xdip_l1d-ptp
#export VERSION=xdip_l2c-ptp
#export CHAMPSIM_BIN=champsim_opt_pref_l1i-128-12_${VERSION}
export CONF_TAG=opt_pref
export CHAMPSIM_BIN=champsim_${CONF_TAG}_${VERSION}

export BUILD_CHAMPSIN=false

# xdip settings
export XDIP_INSTR_POS=0
export XDIP_DATA_POS=2
export XDIP_MAX_LRU=6
export PTP_ENABLE_THRESHOLD=1.0

# EXPERIMENT DESCRIPTOR TAG 
#export DESCR_TAG="_xdip_i${XDIP_INSTR_POS}_d${XDIP_DATA_POS}"
#export DESCR_TAG="_simple_pref_imru_facc_dlru_dip${XDIP_DATA_POS}_maxlru${XDIP_MAX_LRU}"
#export DESCR_TAG="_opt_pref_stlb:dip_l1d:ptp@${PTP_ENABLE_THRESHOLD}"
#export DESCR_TAG="_opt_pref_xdip_datapos${XDIP_DATA_POS}_maxlru${XDIP_MAX_LRU}"
#export DESCR_TAG="_opt_pref_perfectSTLB"
export DESCR_TAG="_opt_pref_l1i-128-12_${VERSION}"

