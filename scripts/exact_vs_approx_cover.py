import time
import subprocess
import os
import json
import numpy as np
import glob

def main():
	data_sets = [("/users/eng/eliavb/data/youtube", False, "youtube", "/work/eng/eliavb/experiments/youtube/cover", "/work/eng/eliavb/youtube_64")]
	
	accuracy = "64"
	arguments = ["--K=" + accuracy, "--num_threads=", "--directed="]
	graph_dir_base = "--graph_dir="
	min_influences = [100, 1000, 5000, 10000, 50000]
	Ts = [100, 1000, 10000]
	threads = 14
	output = ""
	for data_set in data_sets:
		for T in Ts:
			for min_influence in min_influences:
				sketch_file = "--sketch_file=" + data_set[4]
				K = arguments[0]
				thread_ = arguments[1] + str(threads)
				graph_dir = graph_dir_base + data_set[0]
				directed = arguments[2]
				name = data_set[2]
				T_ = "--T=" + str(T)
				min_influence_for_seed_set = "--min_influence_for_seed_set=" + str(min_influence)
				if (data_set[1]):
					directed += "true"
				else:
					directed += "false"
				
				o_file = "/work/eng/eliavb/experiments/" + name + "/cover/" + name +"_" + str(T) + "_" + str(min_influence) + "_" + accuracy
				output = "--output_file=" + o_file
				if (os.path.exists(o_file)):
					continue
				command = "/work/eng/eliavb/install/bin/t_skim_reverse_rank_app %s %s %s %s %s %s %s %s" % (K, thread_, graph_dir, directed, output, T_, min_influence_for_seed_set, sketch_file)
				print command
				process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
				process.wait()
	
	for data_set in data_sets:
		for T in Ts:
			graph_dir = graph_dir_base + data_set[0]
			directed = arguments[2]
			name = data_set[2]
			T_ = "--T=" + str(T)
			if (data_set[1]):
				directed += "true"
			else:
				directed += "false"
			
			o_file = "/work/eng/eliavb/experiments/" + name + "/cover/" + name +"_" + str(T)
			output = "--output_file=" + o_file
			command = "/work/eng/eliavb/install/bin/t_skim_exact_app %s %s %s %s" % (graph_dir, directed, output, T_)
			print command
			process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
			process.wait()		

if __name__ == '__main__':
	main()