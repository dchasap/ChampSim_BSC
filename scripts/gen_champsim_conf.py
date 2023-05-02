#!/usr/bin/env python3
import json
import sys,os
import itertools
import functools
import operator
import copy
from collections import ChainMap


def load_config(filename):
  #config_file = open(filename)
	#config = json.load(config_file, 'r')
	config_file = open(filename)
	config = json.load(config_file)
	return config


def save_config(config, filename):
	#print(config_file['L1D']['sets'])
	output_file = open(filename, 'w')
	output_file.write(json.dumps(dict(config), indent=2))


def create_copy(config):
	#return config.copy()
	return copy.deepcopy(config)


def set_entry(config, context, entry, value):
	if context is None:
		config[entry] = value
	else:
		config[context][entry] = value



### Main ###

conf_tag = 'dev' 

if len(sys.argv) >= 2:

	config = load_config(sys.argv[1])

	# create huge-stlb config
	new_config = create_copy(config)
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_huge-stlb')
	set_entry(new_config, 'STLB', 'sets', 2048)
	set_entry(new_config, 'STLB', 'ways', 48)
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_huge-stlb.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_huge-stlb.json')	

	# create perfect-stlb config
	new_config = create_copy(config)
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_perfect-stlb')
	set_entry(new_config, 'STLB', 'force_hit', True)
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_perfect-stlb.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_perfect-stlb.json')	

	# create perfect-istlb config
	new_config = create_copy(config)
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_perfect-istlb')
	set_entry(new_config, 'STLB', 'force_hit', True)
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_perfect-istlb.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_perfect-istlb.json')	

	# create perfect-dtl1d config
	new_config = create_copy(config)
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_perfect-dtl1d')
	set_entry(new_config, 'L1D', 'force_hit', True)
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_perfect-dtl1d.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_perfect-dtl1d.json')	

	# create perfect-dtl1d config
	new_config = create_copy(config)
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_perfect-istlb-dtl1d')
	set_entry(new_config, 'STLB', 'force_hit', True)
	set_entry(new_config, 'L1D', 'force_hit', True)
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_perfect-istlb-dtl1d.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_perfect-istlb-dtl1d.json')	

	# create xdip config
	new_config = create_copy(config)
	label = 'stlb-xdip'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'STLB', 'replacement', 'xdip')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create stlb-xdip_perfect-dtl1d config
	new_config = create_copy(config)
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_stlb-xdip_perfect-dtl1d')
	set_entry(new_config, 'STLB', 'replacement', 'xdip')
	set_entry(new_config, 'L1D', 'force_hit', True)
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_stlb-xdip_perfect-dtl1d.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_stlb-xdip_perfect-dtl1d.json')	

	# create xxxdip config
	new_config = create_copy(config)
	label = 'stlb-xxxdip'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'STLB', 'replacement', 'xxxdip')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create xxxdip config
	new_config = create_copy(config)
	label = 'stlb-xxxdip_perfect-dtl1d'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'STLB', 'replacement', 'xxxdip')
	set_entry(new_config, 'L1D', 'force_hit', True)
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create xxxdip config
	new_config = create_copy(config)
	label = 'stlb-xdip_l1d-keepDT'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'STLB', 'replacement', 'xdip')
	set_entry(new_config, 'L1D', 'replacement', 'lru_ptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create xxxdip config
	new_config = create_copy(config)
	label = 'l1d-keepDT'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'L1D', 'replacement', 'lru_ptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create lru_orig config
	new_config = create_copy(config)
	label = 'lru_orig'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'L1D', 'replacement', 'lru_orig')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create lru_orig config
	new_config = create_copy(config)
	label = 'lru'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'L1D', 'replacement', 'lru')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	


else:
	print("Base configuration filename is required!")

'''
	# create perfect-itlb-only config
	new_config = create_copy(config)
	set_entry(new_config, None, 'executable_name', 'bin/champsim_' + conf_tag + '_perfect-itlb-only')
	set_entry(new_config, 'ITLB', 'always_hit', True)
	set_entry(new_config, 'ITLB', 'sets', 32)
	save_config(new_config, 'conf/champsim_' + conf_tag  +  '_perfect-itlb-only.json')
	print("Generated conf/champsim_" + conf_tag + '_perfect-itlb-only.json')	

	# create itlb-only config
	new_config = create_copy(config)
	set_entry(new_config, None, 'executable_name', 'bin/champsim_' + conf_tag + '_itlb-only')
	set_entry(new_config, 'ITLB', 'sets', 32)
	save_config(new_config, 'conf/champsim_' + conf_tag  +  '_itlb-only.json')
	print("Generated conf/champsim_" + conf_tag + '_itlb-only.json')	
	
# create itlb-only_perfect-dstlb config
	new_config = create_copy(config)
	set_entry(new_config, None, 'executable_name', 'bin/champsim_' + conf_tag + '_itlb-only_perfect-dstlb')
	set_entry(new_config, 'STLB', 'always_hit', True)
	set_entry(new_config, 'ITLB', 'sets', 32)
	save_config(new_config, 'conf/champsim_' + conf_tag  +  '_itlb-only_perfect-dstlb.json')
	print("Generated conf/champsim_" + conf_tag + '_itlb-only_perfect-dstlb.json')	


	# create l1d-victim-dpte 
	new_config = create_copy(config)
	set_entry(new_config, None, 'executable_name', 'bin/champsim_' + conf_tag + '_l1d-victim-dpte')
	set_entry(new_config, 'L1D', 'always_hit', True)
	save_config(new_config, 'conf/champsim_' + conf_tag  +  '_l1d-victim-dpte.json')
	print("Generated conf/champsim_" + conf_tag + '_l1d-victim-dpte.json')	
	
	# create stlb-xxxdip_l1d-victim-dpte 
	new_config = create_copy(config)
	set_entry(new_config, None, 'executable_name', 'bin/champsim_' + conf_tag + '_stlb-xxxdip_l1d-victim-dpte')
	set_entry(new_config, 'L1D', 'always_hit', True)
	set_entry(new_config, 'STLB', 'replacement', 'xxxdip')
	save_config(new_config, 'conf/champsim_' + conf_tag  +  '_stlb-xxxdip_l1d-victim-dpte.json')
	print("Generated conf/champsim_" + conf_tag + '_stlb-xxxdip_l1d-victim-dpte.json')	
'''
