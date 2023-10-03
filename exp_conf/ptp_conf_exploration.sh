
# GENERIC CONFIGURATION
EXP_NAME=""
BENCHSUITES="smt_qualcom_srv selected_qualcom_srv spec"
#BENCHSUITES="smt_qualcom_srv selected_qualcom_srv spec_mem"
#BENCHSUITES="spec"

# ITP settings
export ITP_INSTR_POS=0
export ITP_DATA_POS=2
export ITP_MAX_LRU=12
# PTP
export TLB_STRESS_THRESHOLD=10
export PTE_EVICTION_RATIO=1
# XPTP
export MIN_EVICTION_POSITION_L1D=8
export MIN_EVICTION_POSITION_L2C=4

### CONFIGURATION SPACE EXPLORATION ###
## STLB itp ##
#BASE_CONFIGURATIONS="dev_stlb-itp"

#declare -A CONFIGURATIONS=( 
#	['ITP_INSTR_POS']="0" 
#	['ITP_DATA_POS']="2" 
#	['ITP_MAX_LRU']="12" 
#)

#declare -a CONFIGURATION_TAGS=(
#	dev_stlb-itp:{ITP_INSTR_POS}:{ITP_DATA_POS}:{ITP_MAX_LRU}
#)


## L1D *ptp ##
#BASE_CONFIGURATIONS="dev_l1d-dptp dev_l1d-xptp"

#declare -A CONFIGURATIONS=( 
#	['MIN_EVICTION_POSITION']="2 4 6 8 10" 
#)

#declare -a CONFIGURATION_TAGS=(
#	dev_l1d-xptp:{MIN_EVICTION_POSITION}
#)

## L2C *ptp
#BASE_CONFIGURATIONS="dev_stlb-itp_l2c-dptp dev_stlb-itp_l2c-xptp"

#declare -A CONFIGURATIONS=( 
#	['MIN_EVICTION_POSITION']="2 4 6 8 10" 
#)

#declare -a CONFIGURATION_TAGS=(
#	dev_stlb-itp_l2c-dptp:{MIN_EVICTION_POSITION}
#	dev_stlb-itp_l2c-xptp:{MIN_EVICTION_POSITION}
#)

## STLB itp L2C *ptp
#BASE_CONFIGURATIONS="dev_stlb-itp_l2c-dptp dev_stlb-itp_l2c-xptp"

#declare -A CONFIGURATIONS=( 
#	['MIN_EVICTION_POSITION']="2 4 6 8 10" 
#)

#declare -a CONFIGURATION_TAGS=(
#	dev_stlb-itp_l2c-dptp:{MIN_EVICTION_POSITION}
#	dev_stlb-itp_l2c-xptp:{MIN_EVICTION_POSITION}
#)



## L2C ptp ##
#BASE_CONFIGURATIONS="dev_llc-ptp"

#declare -A CONFIGURATIONS=( 
#	['TLB_STRESS_THRESHOLD']="10" 
#	['PTE_EVICTION_RATIO']="1 10 20 50 80 100" 
#)

#declare -a CONFIGURATION_TAGS=(
#	dev_llc-ptp:{TLB_STRESS_THRESHOLD}:{PTE_EVICTION_RATIO}
#)

## STLB itp L1D *ptp
#BASE_CONFIGURATIONS="dev_stlb-itp_l2c-dptp dev_stlb-itp_l2c-xptp"

#declare -A CONFIGURATIONS=( 
#	['MIN_EVICTION_POSITION']="2 4 6 8 10" 
#)

#declare -a CONFIGURATION_TAGS=(
#	dev_stlb-itp_l2c-dptp:{MIN_EVICTION_POSITION}
#	dev_stlb-itp_l2c-dptp:{MIN_EVICTION_POSITION}
#)

## STLB itp L1D+L2C *ptp
BASE_CONFIGURATIONS="dev_l2c-tdrrip_llc-tship"

declare -A CONFIGURATIONS=( 
	['ITP_INSTR_POS']="0"
	['ITP_DATA_POS']="2"
	['ITP_MAX_LRU']="12"
	['MIN_EVICTION_POSITION_L1D']="8" 
	['MIN_EVICTION_POSITION_L2C']="4" 
	['TLB_STRESS_THRESHOLD']="10" 
	['PTE_EVICTION_RATIO']="1" 
)

declare -a CONFIGURATION_TAGS=(
	dev_l2c-drrip_llc-tship
)

## STLB itp L1D+L2C *ptp
#BASE_CONFIGURATIONS="dev_stlb-itp_l1d-dptp_l2c-xptp"

#declare -A CONFIGURATIONS=( 
#	['ITP_INSTR_POS']="0"
#	['ITP_DATA_POS']="2"
#	['ITP_MAX_LRU']="2 4 6 8 10 12"
#	['MIN_EVICTION_POSITION_L1D']="8" 
#	['MIN_EVICTION_POSITION_L2C']="2" 
#)

#declare -a CONFIGURATION_TAGS=(
#	dev_stlb-itp:{ITP_INSTR_POS}:{ITP_DATA_POS}:{ITP_MAX_LRU}_l1d-dptp:{MIN_EVICTION_POSITION_L1D}_l2c-xptp:{MIN_EVICTION_POSITION_L2C}
#)
#export ITP_INSTR_POS=0
#export ITP_DATA_POS=2
#export ITP_MAX_LRU=4

## STLB itp L1D+L2C *ptp
#BASE_CONFIGURATIONS="dev_stlb-chirp"


#declare -A CONFIGURATIONS=( 
#	['ITP_INSTR_POS']="0"
#	['ITP_DATA_POS']="2"
#	['ITP_MAX_LRU']="12"
#	['MIN_EVICTION_POSITION_L1D']="8" 
#	['MIN_EVICTION_POSITION_L2C']="4" 
#	['TLB_STRESS_THRESHOLD']="10" 
#	['PTE_EVICTION_RATIO']="1" 
#)

#declare -a CONFIGURATION_TAGS=(
#	dev_stlb-chirp
#)


BUILD_CHAMPSIM=true

