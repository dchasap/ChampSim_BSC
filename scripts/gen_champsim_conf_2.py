#!/usr/bin/env python3
import json
import sys,os
import itertools
import functools
import operator
import copy
from collections import ChainMap
import re

#dictionary for translating attributes from short to long names
attr_names = { 's':'sets', 'w':'ways', 'r':'replacement', 'p':'prefetcher' }

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

if len(sys.argv) >= 3:

	default_config = load_config(sys.argv[1])

	# create l2c-ship_llc-drrip config
	new_config = create_copy(default_config)
	conf_tag = sys.argv[2]

	print('\nCreating new configuration file with tag ' + conf_tag + "...\n")
	set_entry(new_config, None, 'executable_name', 'champsim_' + conf_tag)

	CACHES = ['DTLB', 'ITLB', 'STLB', 'L1I', 'L1D', 'L2C', 'LLC']	
	for cache in CACHES:
		cache_conf = re.findall( cache.lower() + "-[^_]*", conf_tag)
		#print(cache_conf)
		if len(cache_conf):
			cache_conf = re.split( "-", cache_conf[0])	
			#print(cache_conf)
			cache_conf.pop(0)
			print(cache)
			for attribute in cache_conf:
				attribute = re.split('\.', attribute)
				#print(attribute)
				print("\t" + attr_names[attribute[0]] + ":" + attribute[1])
				try: 
					value = int(attribute[1])
				except:
					value = attribute[1]
				set_entry(new_config, cache, attr_names[attribute[0]], value)

	print("\nSaved new configuration file sim_conf/champsim_" + conf_tag + '.json.\n')
	save_config(new_config, 'sim_conf/champsim_' + conf_tag + '.json')

else:
	print("Base configuration filename and tag are required!\n")

