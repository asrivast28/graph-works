#ifndef LOCAL_ALGORITHMFUNCTION_HPP_
#define LOCAL_ALGORITHMFUNCTION_HPP_
#include <vector>
#include "Graph.hpp"
#include "GraphAlgorithmFunction.hpp"

class LocalComputationAlgorithmFunction: public GraphAlgorithmFunction {
public:
  bool  operator()(
    Graph::Graph& g,
	const CombineFunction& combine,
	const std::vector<std::vector<Graph::Node> >& interactionSet
  ) const = 0{
	  for(int i = 0; i < g.getProcessorNodeList().size(); ++ i) {
			combine(g.getProcessorNodeList()[i], g.getProcessorNodeList()[i]);
		} // for

		return true;
  }
  Graph::AlgorithmChoice getType(){
	  return Graph::LocalComputation;
  }

};

#endif // LOCAL_ALGORITHMFUNCTION_HPP_
