#ifndef SAMPLE_LOCAL_COMBINEFUNCTION_HPP_
#define SAMPLE_LOCAL_COMBINEFUNCTION_HPP_

#include "Graph.hpp"
#include "CombineFunction.hpp"

class SampleLocalCombineFunction : public CombineFunction {
public:
  bool operator()(Graph::Node& u, const Graph::Node& v) const {
	  if(u.isLeaf()) {
		  //TODO
	  } else {
		  //TODO
	  }
	  return true;
  }

}; // class CombineFunction

#endif // SAMPLE_LOCAL_COMBINEFUNCTION_HPP_
