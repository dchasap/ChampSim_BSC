#!/usr/bin/python3

"""
__name__ = 
__author__ =
__description = Accepts n number of simpoints' data and corresponding 
								weights in a file and produces the summed data
"""

import argparse	
import pandas as pd
import numpy as np
import csv

### Command Line Arguments ###
parser = argparse.ArgumentParser()
parser.add_argument('--input-files', dest='input_files', required=True, default=None, nargs='+', help="List of csv files, each corresponding to a simpoint of a benchmark application")
parser.add_argument('--weight-file', dest='weight_file', required=True, default=None, help='File containing the weights of the corresponding simpoint csv files')
parser.add_argument('--exclude-columns', dest='exclude_cols', required=False, default=list(), nargs='+', help='List of columns to exclude from weights (fist df\'s values are gonna be preserved)')
parser.add_argument('--output-file', dest='output_file', required=False, default="data.csv", help="Output file name")

### Main ###
args = parser.parse_args()

assert(len(args.input_files) > 0)

# first load the weights
weights = []
weight_file = open(args.weight_file)
for line in weight_file:
	weights.append(float(line))

# loading csv data #
final_df = pd.DataFrame()
weight_index = 0
for input_file in args.input_files:

	df = pd.read_csv(input_file, sep=',')
	columns = df.columns.values.tolist()
	print(input_file)
	print(df.head(5))

	if final_df.empty:
		final_df = df
		# remove columns that should not get weights applied
		for col in args.exclude_cols:
			columns.remove(col)		
		for col in columns:
			final_df[col] = final_df[col] * weights[weight_index]
		weight_index += 1
		continue

	# remove columns that should not get weights applied
	for col in args.exclude_cols:
		columns.remove(col)		

	# get df columns so we can iterate over all of them
	for col in columns:
		final_df[col] = final_df[col] + (df[col] * weights[weight_index])

	weight_index += 1

# apply denominator
for col in columns:
	final_df[col] = final_df[col] / sum(weights)

print(final_df.head(5))
final_df.to_csv(args.output_file, index=False)











