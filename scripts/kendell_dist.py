import time
import subprocess
import os
import json
import numpy as np

def main():
	data_sets = [("/users/eng/eliavb/data/slashdot", True)]
	accuracy = "128"
	arguments = ["--K=" + accuracy, "--num_threads=", "--directed="]
	graph_dir_base = "--graph_dir="
	
	threads = 14
	output = ""
	for data_set in data_sets:
		K = arguments[0]
		thread_ = arguments[1] + str(threads)
		graph_dir = graph_dir_base + data_set[0]
		directed = arguments[2]
		if (data_set[1]):
			directed += "true"
		else:
			directed += "false"
		
		o_file = "/work/eng/eliavb/slashdot" + "_" + accuracy
		output = "--output_file=" + o_file
		command = "/work/eng/eliavb/install/bin/sketch_app %s %s %s %s %s " % (K, thread_, graph_dir, directed, output)
		if (os.path.exists(o_file) == False):
			print command
			process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
			process.wait()

	num_nodes = 1000
	for i in xrange(num_nodes):
		for data_set in data_sets:
			random_node = np.random.randint(70000)
			source = "--source=" + str(random_node)
			directed = "--directed="
			graph_dir = graph_dir_base + data_set[0]
			output_file_dijkstra = "--output_file=/work/eng/eliavb/experiments/slashdot/" + str(random_node) + "_d"
			output_file_reverse_rank = "--output_file=/work/eng/eliavb/experiments/slashdot/" + str(random_node) + "_r"
			if (data_set[1]):
				directed += "true"
			else:
				directed += "false"
			command_dijkstra = "/work/eng/eliavb/install/bin/dijkstra_app %s %s %s %s" % (source, directed, graph_dir, output_file_dijkstra)
			process = subprocess.Popen(command_dijkstra, shell=True, stdout=subprocess.PIPE)
			process.wait()

			K = "--K=128"
			sketch_file = "--sketch_file=" + o_file
			command_reverse_rank = "/work/eng/eliavb/install/bin/reverse_rank_app %s %s %s %s %s %s" % (source, directed, graph_dir, output_file_reverse_rank, sketch_file, K)
			process = subprocess.Popen(command_reverse_rank, shell=True, stdout=subprocess.PIPE)
			process.wait()

if __name__ == '__main__':
	main()