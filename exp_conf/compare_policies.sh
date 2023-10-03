# GENERIC CONFIGURATION
EXP_NAME=""
BENCHSUITES="smt_qualcom_srv selected_qualcom_srv"
BENCHSUITES="spec"
#BENCHSUITES="qualcom_server"
#BENCHSUITES="test"

# ITP settings
export ITP_INSTR_POS=0
export ITP_DATA_POS=2
export ITP_MAX_LRU=4
export ITP_MIN_EVICTION_POSITION=4
# PTP
export TLB_STRESS_THRESHOLD=999999
#export PTE_EVICTION_RATIO=50
# XPTP
#export MIN_EVICTION_POSITION=4
#export MIN_EVICTION_POSITION_L1D=10
#export MIN_EVICTION_POSITION_L2C=4
#BENCHSUITES="qualcom_srv"
#BENCHSUITES="test"

### CONFIGURATION SPACE EXPLORATION ###


# state-of-the-art comparison
BASE_CONFIGURATIONS="
dev_l2c-r.drrip_llc-r.ship
dev_llc-r.ship
dev_llc-r.mockingjay
dev_l2c-r.tdrrip_llc-r.tship
dev_l2c-r.ptp_llc-r.ptp
dev_l2c-r.ptp
dev_stlb-r.chirp
"

declare -A CONFIGURATIONS=( 
	['ITP_INSTR_POS']="0" 
	['ITP_DATA_POS']="2" 
	['ITP_MAX_LRU']="12" 
	['MIN_EVICTION_POSITION']="4" 
	['MIN_EVICTION_POSITION_L1D']="8" 
	['MIN_EVICTION_POSITION_L2C']="4" 
	['TLB_STRESS_THRESHOLD']="10" 
	['PTE_EVICTION_RATIO']="1" 
)

BASE_CONFIGURATIONS="
dev_llc-s.4096-w.16.csv
dev_llc-s.2048-w.16.csv
dev_llc-s.1024-w.16.csv
dev_llc-s.512-w.16.csv
dev_stlb-r.itp_l1d-r.dptp_l2c-r.xptp_llc-s.4096-w.16.csv
dev_stlb-r.itp_l1d-r.dptp_l2c-r.xptp_llc-s.2048-w.16.csv
dev_stlb-r.itp_l1d-r.dptp_l2c-r.xptp_llc-s.1024-w.16.csv
dev_stlb-r.itp_l1d-r.dptp_l2c-r.xptp_llc-s.512-w.16.csv
"


BASE_CONFIGURATIONS="
dev_llc-r.mockingjay
dev_llc-r.ship
dev_l2c-r.drrip_llc-r.ship
dev_l2c-r.tdrrip_llc-r.tship
dev_l2c-r.tdrrip
dev_l2c-r.ptp_llc-r.ptp
dev_l2c-r.ptp
dev_stlb-r.itp_l2c-r.dptp_l2c-r.xptp
"

BASE_CONFIGURATIONS="
dev_stlb-r.itp_l1d-r.dptp_l2c-r.dptp
dev_stlb-r.itp_l1d-r.dptp_l2c-r.xptp
dev_stlb-r.itp_l1d-r.xptp_l2c-r.dptp
dev_stlb-r.itp_l1d-r.xptp_l2c-r.xptp
"

BASE_CONFIGURATIONS="
dev_llc-s.2048-w.16
dev_llc-s.1024-w.16
dev_llc-s.512-w.16
dev_stlb-r.itp_l2c-r.xptp_llc-s.2048-w.16
dev_stlb-r.itp_l2c-r.xptp_llc-s.1024-w.16
dev_stlb-r.itp_l2c-r.xptp_llc-s.512-w.16
"

BASE_CONFIGURATIONS="
dev_baseline_recallDist
"

BASE_CONFIGURATIONS="
dev_stlb-r.kit dev_stlb-r.kdt
"

BASE_CONFIGURATIONS="
dev_stlb-r.itp_l1d-r.xptp_l2c-r.xptp
"

#BASE_CONFIGURATIONS="
#dev_baseline
#"


#BASE_CONFIGURATIONS="
#dev_baseline
#"

#declare -a CONFIGURATION_TAGS=(
#	dev_l2c-r.lfu
#)



## STLB itp L2C dptp/xptp/ptp
#BASE_CONFIGURATIONS="
#dev_stlb-itp_l1d-dptp_l2c-xptp 
#dev_stlb-itp_l1d-dptp_l2c-xptp_llc-ship"

#declare -A CONFIGURATIONS=( 
#	['MIN_EVICTION_POSITION_L1D']="8" 
#	['MIN_EVICTION_POSITION_L2C']="4" 
#)

#declare -a CONFIGURATION_TAGS=(
#	dev_stlb-itp_l1d-dptp:{MIN_EVICTION_POSITION_L1D}_l2c-xptp:{MIN_EVICTION_POSITION_L2C}
#	dev_stlb-itp_l1d-dptp:{MIN_EVICTION_POSITION_L1D}_l2c-xptp:{MIN_EVICTION_POSITION_L2C}_llc-ship
#)

# Performance Breakdown
#BASE_CONFIGURATIONS="dev_llc-tship"

#declare -A CONFIGURATIONS=( 
#	['DUMMY']="" 
#)

#declare -a CONFIGURATION_TAGS=(
#	dev_llc-tship
#)



BUILD_CHAMPSIM=true

