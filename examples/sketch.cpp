using namespace all_distance_sketch;

/*
	Calculate an undirected graph sketch with K=64
*/
void calculate_graph_sketch(graph::Graph< graph::TUnDirectedGraph >* graph) {
 	GraphSketch graph_sketch;
	int k_all_distance_sketch = 64;
	graph_sketch.InitGraphSketch(k_all_distance_sketch, graph->GetMxNId());  
	CalculateGraphSketch< graph::TUnDirectedGraph >(graph, &graph_sketch);
}


/*
	Calculate a directed graph sketch with K=64
*/
void calculate_graph_sketch(graph::Graph< graph::TDirectedGraph >* graph) {
 	GraphSketch graph_sketch;
	int k_all_distance_sketch = 64;
	graph_sketch.InitGraphSketch(k_all_distance_sketch, graph->GetMxNId());  
	CalculateGraphSketch< graph::TDirectedGraph >(graph, &graph_sketch);
}


/*
	Calculate graph sketch with 8 threads
*/
void calculate_graph_sketch_multi(graph::Graph< graph::TUnDirectedGraph >* graph) {
	GraphSketch graph_sketch;
	int k_all_distance_sketch = 128;
	graph_sketch.InitGraphSketch(k_all_distance_sketch, graph->GetMxNId());
	CalculateGraphSketchMultiCore< graph::TUnDirectedGraph >(graph, &graph_sketch);
}
/*
	Iteate the Graph Sketch data structure
*/
void iterate_graph_sketch(GraphSketch* graph_sketch) {
  for (auto node_sketch_it = graph_sketch->Begin();
       node_sketch_it != graph_sketch->End();
       node_sketch_it++) {
    std::cout << " node id=" << node_sketch_it->GetNId() <<
                 " random id=" << node_sketch_it->GetRandomId() << std::endl;
    for (auto sketch_content_it = node_sketch_it->Begin();
         sketch_content_it != node_sketch_it->End();
         sketch_content_it++){
      std::cout << " sketch node id=" << sketch_content_it->GetNId() <<
                   " distance=" << sketch_content_it->GetDistance();
    }  
  }
}

/*
	Save the graph Sketch to Gpb
*/
void save_graph_sketch_to_gpb(GraphSketch* graph_sketch) {
	AllDistanceSketchGpb all_distance_sketch_gpb;
	graph_sketch->SaveGraphSketchToGpb(&all_distance_sketch_gpb);
}

/*
	Load graph sketch from Gpb
*/
void load_graph_sketch_from_gpb(AllDistanceSketchGpb* all_distance_sketch_gpb) {
	GraphSketch graph_sketch;
	graph_sketch.LoadGraphSketchFromGpb(*all_distance_sketch_gpb);
}