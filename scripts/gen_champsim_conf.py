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

	# create random config
	new_config = create_copy(config)
	label = 'l1d-rand'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'L1D', 'replacement', 'random')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create l2c-ship_llc-drrip config
	new_config = create_copy(config)
	label = 'l2c-srrip_llc-ship'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'L2C', 'replacement', 'srrip')
	set_entry(new_config, 'LLC', 'replacement', 'ship')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create l2c-drrip config
	new_config = create_copy(config)
	label = 'l2c-srrip'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'L2C', 'replacement', 'srrip')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create llc-ship config
	new_config = create_copy(config)
	label = 'llc-ship'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'LLC', 'replacement', 'ship')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create l2c-ship_llc-drrip config
	new_config = create_copy(config)
	label = 'l2c-drrip_llc-ship'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'L2C', 'replacement', 'drrip')
	set_entry(new_config, 'LLC', 'replacement', 'ship')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create l2c-drrip config
	new_config = create_copy(config)
	label = 'l2c-drrip'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'L2C', 'replacement', 'drrip')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create llc-mockingjay config
	new_config = create_copy(config)
	label = 'llc-mockingjay'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'LLC', 'replacement', 'mockingjay')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create stlb-chirp config
	new_config = create_copy(config)
	label = 'stlb-chirp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'STLB', 'replacement', 'chirp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	


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

	# create itp config
	new_config = create_copy(config)
	label = 'stlb-itp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'STLB', 'replacement', 'itp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create stlb-itp_perfect-dtl1d config
	new_config = create_copy(config)
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_stlb-itp_perfect-dtl1d')
	set_entry(new_config, 'STLB', 'replacement', 'itp')
	set_entry(new_config, 'L1D', 'force_hit', True)
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_stlb-itp_perfect-dtl1d.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_stlb-itp_perfect-dtl1d.json')	

	# create l1d-dptp config
	new_config = create_copy(config)
	label = 'l1d-dptp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'L1D', 'replacement', 'dptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create stlb-ipt_l1d-dptp config
	new_config = create_copy(config)
	label = 'stlb-itp_l1d-dptp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'STLB', 'replacement', 'itp')
	set_entry(new_config, 'L1D', 'replacement', 'dptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create l2c-dptp config
	new_config = create_copy(config)
	label = 'l2c-dptp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'L2C', 'replacement', 'dptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	


	# create stlb-itp_l2c-dptp config
	new_config = create_copy(config)
	label = 'stlb-itp_l2c-dptp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'STLB', 'replacement', 'itp')
	set_entry(new_config, 'L2C', 'replacement', 'dptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create stlb-itp_l1d-dptp_l2c-dptp config
	new_config = create_copy(config)
	label = 'stlb-itp_l1d-dptp_l2c-dptp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'STLB', 'replacement', 'itp')
	set_entry(new_config, 'L1D', 'replacement', 'dptp')
	set_entry(new_config, 'L2C', 'replacement', 'dptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create stlb-itp_l2c-dptp_llc-dptp config
	new_config = create_copy(config)
	label = 'stlb-itp_l2c-dptp_llc-dptp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'STLB', 'replacement', 'itp')
	set_entry(new_config, 'L2C', 'replacement', 'dptp')
	set_entry(new_config, 'LLC', 'replacement', 'dptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create stlb-itp_l1d-dptp_l2c-dptp_llc-dptp config
	new_config = create_copy(config)
	label = 'stlb-itp_l1d-dptp_l2c-dptp_llc-dptp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'STLB', 'replacement', 'itp')
	set_entry(new_config, 'L1D', 'replacement', 'dptp')
	set_entry(new_config, 'L2C', 'replacement', 'dptp')
	set_entry(new_config, 'LLC', 'replacement', 'dptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create l1d-xptp config
	new_config = create_copy(config)
	label = 'l1d-xptp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'L1D', 'replacement', 'xptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create l2c-xptp config
	new_config = create_copy(config)
	label = 'l2c-xptp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'L2C', 'replacement', 'xptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create stlb-itp_l1d-xptp config
	new_config = create_copy(config)
	label = 'stlb-itp_l1d-xptp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'STLB', 'replacement', 'itp')
	set_entry(new_config, 'L1D', 'replacement', 'xptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create stlb-itp_l2c-xptp config
	new_config = create_copy(config)
	label = 'stlb-itp_l2c-xptp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'STLB', 'replacement', 'itp')
	set_entry(new_config, 'L2C', 'replacement', 'xptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create l2c-xptp_llc-xptp config
	new_config = create_copy(config)
	label = 'l2c-xptp_llc-xptp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'L2C', 'replacement', 'xptp')
	set_entry(new_config, 'LLC', 'replacement', 'xptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create stlb-itp_l1d-dptp_l2c-xptp config
	new_config = create_copy(config)
	label = 'stlb-itp_l1d-dptp_l2c-xptp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'STLB', 'replacement', 'itp')
	set_entry(new_config, 'L1D', 'replacement', 'dptp')
	set_entry(new_config, 'L2C', 'replacement', 'xptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create stlb-itp_l1d-xptp_l2c-xptp config
	new_config = create_copy(config)
	label = 'stlb-itp_l1d-xptp_l2c-xptp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'STLB', 'replacement', 'itp')
	set_entry(new_config, 'L1D', 'replacement', 'xptp')
	set_entry(new_config, 'L2C', 'replacement', 'xptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create l2c-ptp_llc-ptp config
	new_config = create_copy(config)
	label = 'l2c-ptp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'L2C', 'replacement', 'ptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create llc-ptp config
	new_config = create_copy(config)
	label = 'llc-ptp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'LLC', 'replacement', 'ptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create l2c-ptp_llc-ptp config
	new_config = create_copy(config)
	label = 'l2c-ptp_llc-ptp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'L2C', 'replacement', 'ptp')
	set_entry(new_config, 'LLC', 'replacement', 'ptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

# create stlb-itp_l2c-ptp config
	new_config = create_copy(config)
	label = 'stlb-itp_l2c-ptp'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'STLB', 'replacement', 'itp')
	set_entry(new_config, 'L2C', 'replacement', 'ptp')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create stlb-itp_l1d-xptp_l2c-xptp config
	new_config = create_copy(config)
	label = 'stlb-itp_l1d-dptp_l2c-xptp_llc-ship'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'STLB', 'replacement', 'itp')
	set_entry(new_config, 'L1D', 'replacement', 'dptp')
	set_entry(new_config, 'L2C', 'replacement', 'xptp')
	set_entry(new_config, 'LLC', 'replacement', 'ship')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create l2c-tddrip config
	new_config = create_copy(config)
	label = 'l2c-tdrrip'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'L2C', 'replacement', 't-drrip')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create llc-tship config
	new_config = create_copy(config)
	label = 'llc-tship'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'LLC', 'replacement', 't-ship')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	


	# create l2c-tdrrip_llc-tship config
	new_config = create_copy(config)
	label = 'l2c-tdrrip_llc-tship'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'L2C', 'replacement', 't-drrip')
	set_entry(new_config, 'LLC', 'replacement', 't-ship')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

	# create stlb-itp_l1d-dptp_l2c-xptp_llc-ship
	new_config = create_copy(config)
	label = 'stlb-itp_l1d-dptp_l2c-xptp_llc-ship'
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag + '_' + label)
	set_entry(new_config, 'STLB', 'replacement', 'itp')
	set_entry(new_config, 'L1D', 'replacement', 'dptp')
	set_entry(new_config, 'L2C', 'replacement', 'xptp')
	set_entry(new_config, 'LLC', 'replacement', 't-ship')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag  +  '_' + label + '.json')
	print("Generated sim_conf/champsim_" + conf_tag + '_' + label + '.json')	

else:
	print("Base configuration filename is required!")

