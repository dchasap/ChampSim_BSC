#!/usr/bin/python3

"""
__name__ = gen_champsim_jobs.py
__author__ = Dimitrios Chasapis
__description = Generate ChampSim binary and submit job script to SLURM
								Requires a configuration file (see exp/example.exp) and
								a file containing a list of benchmarks/simpoints
"""

import argparse	
import configparser
import subprocess
from string import Template

### Modifies ChampSim header files, if necessary, and recompiles ###
def prepare_experiment(config):
	### main parameters ###
	branch_predictor = config['Main']['branch_predictor']
	stlb_replacement = config['Main']['stlb_replacement']

	for section in config:
		if not config.has_option(section, 'header'): continue
		### cache/tlb parameters ###
		header = config[section]['header']

		### Backup files ###
		cmd = "cp " + header + " " + header + "_backup"  
		subprocess.run(cmd, shell=True)

		### Edit files ###
		for key in config[section]:
			if key == "header" or key == "source": continue
			cmd = "sed \'s/#define " + key.upper() + " .*/#define " + key.upper() + " " + config[section][key] + "/\' -i " + header 
			subprocess.run(cmd, shell=True)

	if config.has_option('Experiment', 'binary_extra_option'):
		cmd = "./build_champsim.sh " + branch_predictor + " no no no no lru " + stlb_replacement + " 1 _" + config['Experiment']['binary_extra_option']
	else:
		cmd = "./build_champsim.sh " + branch_predictor + " no no no no lru " + stlb_replacement + " 1"

	subprocess.run(cmd, shell=True)

	for section in config:
		if not config.has_option(section, 'header'): continue
		header = config[section]['header']
		### Restore files ###
		cmd = "mv " + header + "_backup " + header  
		subprocess.run(cmd, shell=True)

### Generates job script and submits it ### 
def run_experiment(config, bench):

	# first check if environment variables are unique or we need to generate 
	# multiple job scripts for each
	env_vars = {}
	if config.has_section('Environment Variables'):
		for option in config['Environment Variables']:
			#print(option.upper() + ":" + config['Environment Variables'][option] + "\n")
			env_vars[option.upper()] =  (config['Environment Variables'][option]).split()
	
#	print("testing  dict")
#	for option in env_vars:
#		print(option)
#		for value in env_vars[option]:
#			print("\t" + value)

	gen_next_exp = True
	while gen_next_exp:

		gen_next_exp = False

		home_dir = config['Experiment']['home_dir']
		short_name = evaluate_name_tags(config['Experiment']['short_name'], env_vars)
		champsim_bin = config['Main']['champsim_binary']
		warmup_instructions = config['Execution']['warmup_instructions']
		simulation_instructions = config['Execution']['simulation_instructions']
		trace = config['Execution']['trace']
		trace_extension = config['Execution']['trace_extension']
		results_file = config['Experiment']['results_file']

		job_script_file = bench + "_" + short_name + ".run"
		#print("Generating " + job_script_file + " job script")
		job_script = open(job_script_file, 'w')

		job_script.write("#!/bin/bash\n")
		job_script.write("#SBATCH -N 1\n")
		job_script.write("#SBATCH -o " + home_dir + "/dump/" + bench + "_" + short_name + ".out\n")
		job_script.write("#SBATCH -J " + bench + "_" + short_name + "\n")
#		job_script.write("#SBATCH --time=4:00:00\n")
#		job_script.write("#SBATCH --constraint=highmem\n")
		if config['Experiment'].getboolean('enable_debug'):
			job_script.write("#SBATCH --qos=debug\n")
		else:
			job_script.write("#SBATCH --qos=bsc_cs\n")

		job_script.write("module load python/2.7.13\n")
		job_script.write("export bench=" + bench + "\n")

		for option in env_vars:
			if len(env_vars[option]) > 1:
				gen_next_exp = True
				value = env_vars[option].pop()
			else:
				value = env_vars[option][0]
			job_script.write("export " + option.upper() + "=" + value + "\n") 

		if config['Experiment'].getboolean('enable_debug'):
			job_script.write("(gdb -batch -ex \"r\" -ex \"bt\" -ex \"q\" --args " + champsim_bin + " -warmup_instructions " + warmup_instructions + " -simulation_instructions " + simulation_instructions + " -traces " + trace +  " -trace-extensions " + trace_extension + ") &> " + results_file)
		else:
			job_script.write("(" + champsim_bin + " -warmup_instructions " + warmup_instructions + " -simulation_instructions " + simulation_instructions + " -traces " + trace + " -trace-extensions " + trace_extension + ") &> " + results_file)

		job_script.close()

		#print("Submitting" + job_script_file + " job script")
		cmd = "sbatch " + job_script_file
		subprocess.run(cmd, shell=True)
		if not config['Experiment'].getboolean('enable_debug'):
			cmd = "rm " + job_script_file
			subprocess.run(cmd, shell=True)


# replaces ${var} with var value
def evaluate_name_tags(name, env_vars):
	# first prepare a dictionary with a single value for each
	# entry (instead of list)
	var_dict = {}
	for option in env_vars:
		var_dict[option] = env_vars[option][0]

	name_template = Template(name)
	name = name_template.safe_substitute(**var_dict)

	return name


### Main ###
parser = argparse.ArgumentParser()
parser.add_argument('--config', '-c', dest='config_file', required=True, default=None, help="Experiment configuration file")
parser.add_argument('--benchmarks', '-b', dest='benchmarks_file', required=True, default=None, help="File with a list of benchmarks to run the experiment")
parser.add_argument('--skip-build', '-s', dest='build_champsim', required=False, action='store_false')

args = parser.parse_args()
config = configparser.ConfigParser(interpolation=configparser.ExtendedInterpolation())
config.read(args.config_file)

if args.build_champsim:
	build_champsim(args.config_file)

benchmarks = open(args.benchmarks_file, 'r')
for bench in benchmarks:
	bench = bench.strip()
	job_file = submit_job(bench)

