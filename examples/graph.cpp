
using namespace all_distance_sketch::graph;
/*
	The EXPECT_TRUE Macro are here just to help you understand what is the expected result.
*/


/* 
	How to build an undirected graph
*/
void build_undirected_unweighted_graph() {
  Graph< TUnDirectedGraph > graph;
  int count = 100;
  for (int i = 0; i < count; ++i) {
    graph.AddNode(i);
  }
  
  for (int i = 0; i < count-1; ++i) {
    graph.AddEdge(i, i+1);
  }

  for (int i = 0; i < count-1; ++i) {
    EXPECT_TRUE(graph.IsEdge(i, i+1));
    EXPECT_TRUE(graph.IsEdge(i+1, i));
  }
}


/* 
	How to build a directed graph
*/
void build_directed_unweighted_graph() {
  Graph< TUnDirectedGraph > graph;
  int count = 100;
  for (int i = 0; i < count; ++i) {
    graph.AddNode(i);
  }
  
  for (int i = 0; i < count-1; ++i) {
    graph.AddEdge(i, i+1);
  }

  for (int i = 0; i < count-1; ++i) {
    EXPECT_TRUE(graph.IsEdge(i, i+1));
  }
}


/* 
	How to build an undirected weighted graph
*/
void build_undirected_weighted_graph() {
  Graph< TUnDirectedGraph > graph;
  int count = 100;
  for (int i = 0; i < count; ++i) {
    graph.AddNode(i);
  }
  
  for (int i = 0; i < count-1; ++i) {
    graph.AddEdge(i, i+1, i);
  }
  
  for (int i = 0; i < count-1; ++i)
  {
     EXPECT_TRUE(graph.IsEdge(i, i+1));
     EXPECT_TRUE(graph.IsEdge(i+1, i));
     std::pair<bool, EdgeWeight> res = graph.GetEdgeWeight(i, i+1);
     EXPECT_TRUE(res.first);
     EXPECT_EQ(res.second, i);
     // Check that the undirected graph 
     std::pair<bool, EdgeWeight> res1 = graph.GetEdgeWeight(i+1, i);
     EXPECT_TRUE(res1.first);
     EXPECT_EQ(res1.second, i);
  }
}


/*
	Loading the graph from a directory
	The format of the files is documented in the Class graph::Graph
*/
void load_graph_from_dir() {
	std::string path = "some path to dir";
	Graph< TUnDirectedGraph > graph;
	graph.LoadGraphFromDir(path);
}


/*
	Iterating the graph
*/
void iterate_graph(Graph<T>* graph) {
	for (T::TNodeI nodeItr = graph->BegNI();  nodeItr != graph->EndNI() ; nodeItr++ ){
	    int node_id = nodeItr.GetId();
	}
}