#ifndef GRAPHWORKS_ALGORITHMFUNCTION_HPP_
#define GRAPHWORKS_ALGORITHMFUNCTION_HPP_

#include "Graph.hpp"

#include <vector>

class GraphAlgorithmFunction {
public:
  virtual
  bool
  operator()(
    Graph&,
    const CombineFunction&,
    const std::vector<std::vector<Graph::Node> >&
  ) const = 0;

  virtual
  float
  getScore(
    Graph&,
  	const std::vector<std::vector<Graph::Node> >&
  ) const = 0;

  virtual
  Graph::AlgorithmChoice 
  getType() const = 0;
}; // class GraphAlgorithmFunction

#endif // GRAPHWORKS_ALGORITHMFUNCTION_HPP_
