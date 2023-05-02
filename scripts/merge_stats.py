#!/usr/bin/python3

"""
__name__ = 
__author__ =
__description = Appends two dataframes together
"""

import argparse	
import pandas as pd
import numpy as np
import csv

### Command Line Arguments ###
parser = argparse.ArgumentParser()
parser.add_argument('--suffices', dest='suffices', required=True, default=None, nargs='+', help="List of suffices for each df merged")
parser.add_argument('--input-files', dest='input_files', required=True, default=None, nargs='+', help="List of csv files, each corresponding to a simpoint of a benchmark application")
parser.add_argument('--output-file', dest='output_file', required=False, default="data.csv", help="Output file name")

args = parser.parse_args()

assert(len(args.input_files) > 0)
print(args.suffices)

final_df = pd.DataFrame()
for input_file in args.input_files:

	df = pd.read_csv(input_file, sep=',')
	suffix = "_" + args.suffices.pop(0)

	if final_df.empty:
		print("First Iteration")
		benchmarks = df['benchmark'].values.tolist()
		df = df.add_suffix(suffix)
		final_df = df
		continue
	
	print("-------- orig final ---------")	
	print(final_df.head(5))
	print("--------  orig new  ---------")	
	print(df.head(5))
	df = df.drop('benchmark', 1)
	df = df.add_suffix(suffix)
	final_df = final_df.join(df)
	print("-------   merged   ---------")	
	print(final_df.head(5))


final_df['benchmark'] = benchmarks
print(final_df.head(5))
#print(args.row_names)
#final_df['benchmark'] = args.row_names
#print(final_df.head(5))
final_df.to_csv(args.output_file, index=False)





