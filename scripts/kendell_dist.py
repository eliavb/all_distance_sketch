import time
import subprocess
import os
import json
import numpy as np
import glob

def main():
	# ("/users/eng/eliavb/data/slashdot", True, "slashdot")
	data_sets = [("/users/eng/eliavb/data/youtube", False, "youtube", 1000000, "/work/eng/eliavb/experiments/youtube/youtube_nodes")]
	accuracy = "64"
	arguments = ["--K=" + accuracy, "--num_threads=", "--directed="]
	graph_dir_base = "--graph_dir="
	
	threads = 14
	output = ""
	for data_set in data_sets:
		K = arguments[0]
		thread_ = arguments[1] + str(threads)
		graph_dir = graph_dir_base + data_set[0]
		directed = arguments[2]
		name = data_set[2]
		if (data_set[1]):
			directed += "true"
		else:
			directed += "false"
		
		o_file = "/work/eng/eliavb/" + name + "_" + accuracy +"/" + name
		output = "--output_file=" + o_file
		command = "/work/eng/eliavb/install/bin/sketch_app %s %s %s %s %s " % (K, thread_, graph_dir, directed, output)
		files = glob.glob(o_file+"_*")
		if (len(files) == 0):
			print command
			process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
			process.wait()

	for data_set in data_sets:
		K = arguments[0]
		thread_ = arguments[1] + str(threads)
		graph_dir = graph_dir_base + data_set[0]
		directed = arguments[2]
		name = data_set[2]
		if (data_set[1]):
			directed += "true"
		else:
			directed += "false"
		sketch_file = "--sketch_file=" + o_file
		output_dir = "--output_dir=/work/eng/eliavb/experiments/" +name+"/"

		sources = "--sources=" + data_set[4]
		command_reverse_rank = "/work/eng/eliavb/install/bin/reverse_rank_batch_app " + \
									("%s %s %s %s %s %s %s" % (K, thread_, graph_dir, directed, sketch_file, output_dir, sources ))
	
		
		print command_reverse_rank
		process = subprocess.Popen(command_reverse_rank, shell=True, stdout=subprocess.PIPE)
		process.wait()

	for data_set in data_sets:
		with open(data_set[4], 'r') as f:
			for line in f:
				random_node = line.strip()
				source = "--source=" + str(random_node)
				directed = "--directed="
				graph_dir = graph_dir_base + data_set[0]
				name = data_set[2]
				output_file_dijkstra = "--output_file=/work/eng/eliavb/experiments/"+name+"/" + str(random_node) + "_d"
				output_file_reverse_rank = "--output_file=/work/eng/eliavb/experiments/"+name+"/" + str(random_node) + "_r"
				if (data_set[1]):
					directed += "true"
				else:
					directed += "false"
				command_dijkstra = "/work/eng/eliavb/install/bin/dijkstra_app %s %s %s %s" % (source, directed, graph_dir, output_file_dijkstra)
				print command_dijkstra
				process = subprocess.Popen(command_dijkstra, shell=True, stdout=subprocess.PIPE)
				process.wait()

if __name__ == '__main__':
	main()