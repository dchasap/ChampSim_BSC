
# GENERIC CONFIGURATION
#export BENCHSUITE=top10_smt_qualcom_srv
#export BENCHSUITE=selected_qualcom_srv
#export BENCHSUITE=test
#export CONF_TAG=dev_perfect-istlb
#export CONF_TAG=test
#export CHAMPSIM_BIN=champsim_${CONF_TAG}
#export CHAMPSIM_BIN=champsim_default_perfect-istlb
#export CHAMPSIM_BIN=champsim


# this is a list of experiments to run 
EXP_NAME=""
BENCHSUITES="smt_qualcom_srv"
#BENCHSUITES="test"
# xdip settings
export XDIP_INSTR_POS=0
export XDIP_DATA_POS=2
export XDIP_MAX_LRU=6
export PTP_ENABLE_THRESHOLD=1.0
export MIN_EVICTION_POSITION=8
#BENCHSUITES="qualcom_srv"
#BENCHSUITES="test"
#EXPERIMENTS="dev_baseline dev_huge-stlb dev_perfect-stlb dev_perfect-istlb"
#EXPERIMENTS="dev_perfect-dtl1d dev_perfect-istlb-dtl1d"
#EXPERIMENTS="dev_stlb-xdip_perfect-dtl1d"
#EXPERIMENTS="dev_stlb-xxxdip_perfect-dtl1d"
#EXPERIMENTS="dev_stlb-xdip"
#EXPERIMENTS="dev_l1d-keepDT:${MIN_EVICTION_POSITION}"
#EXPERIMENTS="dev_stlb-xdip_l1d-ret:${MIN_EVICTION_POSITION}"
#EXPERIMENTS="dev_stlb-xdip_l1d-ret:${MIN_EVICTION_POSITION}"
#EXPERIMENTS="dev_l1d-ret:${MIN_EVICTION_POSITION}"
#EXPERIMENTS="dev_lru_orig"
#EXPERIMENTS="dev_lru"
EXPERIMENTS="dev_stlb-xdip_l1d-keepDT"
#cp bin/champsim_dev_l1d-keepDT bin/champsim_dev_l1d-ret:${MIN_EVICTION_POSITION}
#cp bin/champsim_dev_l1d-keepDT bin/champsim_dev_l1d-ret:${MIN_EVICTION_POSITION}
#cp bin/champsim_dev_stlb-xdip_l1d-keepDT bin/champsim_dev_stlb-xdip_l1d-ret:${MIN_EVICTION_POSITION}
cp bin/champsim_dev_stlb-xdip_l1d-keepDT bin/champsim_dev_stlb-xdip_l1d-keepDT_me:${MIN_EVICTION_POSITION}
BUILD_CHAMPSIM=false
