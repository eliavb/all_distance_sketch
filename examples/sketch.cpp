using namespace all_distance_sketch;
/*
 Calculate an undirected graph sketch with K=64
*/
void how_to_calculate_graph_sketch(graph::Graph< graph::TUnDirectedGraph >* graph) {
 	GraphSketch graph_sketch;
	int k_all_distance_sketch = 64;
	graph_sketch.InitGraphSketch(k_all_distance_sketch, graph->GetMxNId());  
	CalculateGraphSketch< graph::TUnDirectedGraph >(graph, &graph_sketch);
}

/*
 Calculate a directed graph sketch with K=64
*/
void how_to_calculate_graph_sketch(graph::Graph< graph::TDirectedGraph >* graph) {
 	GraphSketch graph_sketch;
	int k_all_distance_sketch = 64;
	graph_sketch.InitGraphSketch(k_all_distance_sketch, graph->GetMxNId());  
	CalculateGraphSketch< graph::TDirectedGraph >(graph, &graph_sketch);
}

/*
 Calculate graph sketch with 8 threads
 Pre:
 graph - the graph is loaded
 graph_sketch - the graph sketch init function was called
*/
void how_to_calculate_graph_sketch_multi(graph::Graph< graph::TUnDirectedGraph >* graph) {
	GraphSketch graph_sketch;
	int k_all_distance_sketch = 64;
	graph_sketch.InitGraphSketch(k_all_distance_sketch, graph->GetMxNId());
	CalculateGraphSketchMultiCore< graph::TUnDirectedGraph >(graph, graph_sketch);
 }