#ifndef SAMPLE_LOCAL_GENERATEFUNCTION_HPP_
#define SAMPLE_LOCAL_GENERATEFUNCTION_HPP_

#include "Graph.hpp"
#include "GenerateFunction.hpp"

#include <iterator>

class SampleLocalGenerateFunction: public GenerateFunction {
public:
  bool
  operator()(
    const Graph&,
    const Graph::Node& node,
    std::back_insert_iterator<std::vector<Graph::Node> >& iteratorList,
    bool& dependencyFlag
  ) const
  {
	  iteratorList = node;
    dependencyFlag = false;
	  return true;
  }

  virtual
  Graph::AlgorithmChoice
  type() const { return Graph::LocalComputation; }

}; // class GenerateFunction

#endif // SAMPLE_LOCAL_GENERATEFUNCTION_HPP_
