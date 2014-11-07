#ifndef GRAPHWORKS_GENERATEFUNCTION_HPP_
#define GRAPHWORKS_GENERATEFUNCTION_HPP_

#include "Graph.hpp"

#include <iterator>

class GenerateFunction {
public:
  virtual
  bool
  operator()(
    const Graph&,
    const Graph::Node&,
    std::back_insert_iterator<std::vector<Graph::Node> >&
  ) = 0;

  virtual
  Graph::AlgorithmChoice
  type() const { return Graph::General; }

}; // class GenerateFunction

#endif // GRAPHWORKS_GENERATEFUNCTION_HPP_
