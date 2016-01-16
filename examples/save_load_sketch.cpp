using namespace all_distance_sketch;
/*
	Example how to save the graph Sketch to Gpb
*/
void save_graph_sketch_to_gpb(GraphSketch* graph_sketch) {
	AllDistanceSketchGpb all_distance_sketch_gpb;
	graph_sketch->SaveGraphSketchToGpb(&all_distance_sketch_gpb);
}
/*
	Example how to load graph sketch from Gpb
*/
void load_graph_sketch_from_gpb(AllDistanceSketchGpb* all_distance_sketch_gpb) {
	GraphSketch graph_sketch;
	graph_sketch.LoadGraphSketchFromGpb(*all_distance_sketch_gpb);
}
