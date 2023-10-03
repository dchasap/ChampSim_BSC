
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
BENCHSUITES="smt_qualcom_srv selected_qualcom_srv"
#BENCHSUITES="smt_qualcom_srv selected_qualcom_srv spec_mem"
#BENCHSUITES="top10_qualcom_srv"
#BENCHSUITES="selected_qualcom_srv"
#BENCHSUITES="gap"
#BENCHSUITES="spec"
#BENCHSUITES="test"
# ITP settings
export ITP_INSTR_POS=0
export ITP_DATA_POS=2
export ITP_MAX_LRU=4
export ITP_MIN_EVICTION_POSITION=4
# PTP
export TLB_STRESS_THRESHOLD=10
export PTE_EVICTION_RATIO=100
# XPTP
export MIN_EVICTION_POSITION=4
#export MIN_EVICTION_POSITION_L1D=10
#export MIN_EVICTION_POSITION_L2C=4
#BENCHSUITES="qualcom_srv"
#BENCHSUITES="test"

#EXPERIMENTS="dev_baseline"
#EXPERIMENTS="dev_l1d-rand"
#EXPERIMENTS="dev_stlb-xdip"
#EXPERIMENTS="dev_stlb-itp_l2c-xptp:${MIN_EVICTION_POSITION} dev_stlb-itp_l1d-xptp:${MIN_EVICTION_POSITION}_l2c-xptp:${MIN_EVICTION_POSITION}"
#EXPERIMENTS="dev_l1d-dptp dev_l1d-xptp"
EXPERIMENTS="dev_l2c-dptp dev_l2c-xptp"
#EXPERIMENTS="dev_stlb-itp_l2c-xptp dev_stlb-itp_l1d-xptp_l2c-xptp"
#EXPERIMENTS="dev_stlb-itp_l2c-dptp dev_stlb-itp_l1d-dptp_l2c-dptp"
#EXPERIMENTS="dev_stlb-itp_l1d-dptp_l2c-xptp"
#EXPERIMENTS="dev_stlb-itp:${ITP_MIN_EVICTION_POSITION}_l1d-dptp_l2c-xptp"
### CONFIGURATION SPACE EXPLORATION ###
#EXPERIMENTS="dev_stlb-itp:${ITP_MIN_EVICTION_POSITION}_l1d-dptp_l2c-dptp"
#EXPERIMENTS="dev_l2c-ptp:${TLB_STRESS_THRESHOLD}:${PTE_EVICTION_RATIO} dev_l2c-ptp:${TLB_STRESS_THRESHOLD}:${PTE_EVICTION_RATIO}_llc-ptp:${TLB_STRESS_THRESHOLD}:${PTE_EVICTION_RATIO}"
#EXPERIMENTS="dev_l2c-ptp:${TLB_STRESS_THRESHOLD}:${PTE_EVICTION_RATIO}"
#EXPERIMENTS="dev_stlb-itp_l1d-dptp_l2c-dptp"
#EXPERIMENTS="dev_l2c-ptp:${TLB_STRESS_THRESHOLD}:${PTE_EVICTION_RATIO}"
#EXPERIMENTS="dev_stlb-xdip_l1d-keepDT_l2c-keepDT"
#EXPERIMENTS="dev_baseline dev_stlb-xdip dev_stlb-xdip_l1d-keepDT dev_stlb-xdip_l2c-keepDT dev_stlb-xdip_l1d-keepDT_l2c-keepDT"
#EXPERIMENTS="dev_stlb-xdip_l1d-keepDT:${MIN_EVICTION_POSITION} dev_stlb-xdip_l2c-keepDT:${MIN_EVICTION_POSITION}"
#EXPERIMENTS="dev_stlb-xdip_l1d-keepDT:${MIN_EVICTION_POSITION_L1D}_l2c-keepDT:${MIN_EVICTION_POSITION_L2C}"
#EXPERIMENTS="dev_stlb-xdip_l1d-keepDT_llc-keepDT dev_stlb-xdip_l2c-keepDT_llc-keepDT dev_stlb-xdip_l1d-keepDT_l2c-keepDT_llc-keepDT"
#EXPERIMENTS="dev_baseline dev_huge-stlb dev_perfect-stlb dev_perfect-istlb"
#EXPERIMENTS="dev_perfect-dtl1d dev_perfect-istlb-dtl1d"
#EXPERIMENTS="dev_stlb-xdip_perfect-dtl1d"
#EXPERIMENTS="dev_stlb-xxxdip_perfect-dtl1d"
#EXPERIMENTS="dev_stlb-xdip"
#EXPERIMENTS="dev_stlb-xdip_l1d-keepDT:${MIN_EVICTION_POSITION}"
#EXPERIMENTS="dev_stlb-xdip_l1d-keepDT"
#EXPERIMENTS="dev_l1d-keepDT"
#EXPERIMENTS="dev_stlb-xdip dev_stlb-xdip_l1d-ret:${MIN_EVICTION_POSITION}"
#EXPERIMENTS="dev_baseline dev_perfect-istlb dev_stlb-xdip"
#EXPERIMENTS="dev_stlb-xdip_l1d-ret:${MIN_EVICTION_POSITION} dev_l1d-ret:${MIN_EVICTION_POSITION}"
#EXPERIMENTS="dev_stlb-xdip_l1d-keepDT-me:${MIN_EVICTION_POSITION} dev_l1d-keepDT-me:${MIN_EVICTION_POSITION}"
#EXPERIMENTS="dev_stlb-xdip_l1d-keepDT-me:${MIN_EVICTION_POSITION}"
#EXPERIMENTS="dev_baseline"
#cp bin/champsim_dev_l1d-keepDT bin/champsim_dev_stlb-xdip_stlb-xdip_l1d-ret:${MIN_EVICTION_POSITION}
#cp bin/champsim_dev_stlb-xdip_l1d-keepDT bin/champsim_dev_stlb-xdip_stlb-xdip_l1d-ret:${MIN_EVICTION_POSITION}
#cp bin/champsim_dev_stlb-xdip_l1d-keepDT bin/champsim_dev_stlb-xdip_l1d-keepDT:${MIN_EVICTION_POSITION}
#cp bin/champsim_dev_stlb-xdip_l2c-keepDT bin/champsim_dev_stlb-xdip_l2c-keepDT:${MIN_EVICTION_POSITION}
#cp bin/champsim_dev_stlb-itp_l1d-dptp_l2c-xptp bin/champsim_dev_stlb-itp:${ITP_MIN_EVICTION_POSITION}_l1d-dptp_l2c-xptp
#cp bin/champsim_dev_stlb-itp_l1d-xptp_l2c-xptp bin/champsim_dev_stlb-ITP:${ITP_MIN_EVICTION_POSITION}_l1d-xptp_l2c-dptp
#cp bin/champsim_dev_stlb-itp_l1d-dptp_l2c-dptp bin/champsim_dev_stlb-ITP:${ITP_MIN_EVICTION_POSITION}_l1d-dptp_l2c-dptp
#cp bin/champsim_dev_l2c-ptp bin/champsim_dev_l2c-ptp:${TLB_STRESS_THRESHOLD}:${PTE_EVICTION_RATIO}
#cp bin/champsim_dev_l2c-ptp_llc-ptp bin/champsim_dev_l2c-ptp:${TLB_STRESS_THRESHOLD}:${PTE_EVICTION_RATIO}_llc-ptp:${TLB_STRESS_THRESHOLD}:${PTE_EVICTION_RATIO}
BUILD_CHAMPSIM=true

