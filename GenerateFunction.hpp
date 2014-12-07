#ifndef GRAPHWORKS_GENERATEFUNCTION_HPP_
#define GRAPHWORKS_GENERATEFUNCTION_HPP_

#include "Graph.hpp"

#include <iterator>

class DataPoint;

class GenerateFunction {
public:
  /** Implement One of these **/
  virtual
  bool
  operator()(
    const Graph& g,
    const Graph::Node& u,
    std::back_insert_iterator<std::vector<Graph::Node> >& v,
    bool& dependency
  ) const = 0;

  virtual
  bool
  operator()(
    const Graph& g,
    const DataPoint& dataPoint,
    std::back_insert_iterator<std::vector<DataPoint> >& dependentDataPoints,
    bool& dependency
  ) const = 0;

  virtual
  Graph::AlgorithmChoice
  type() const { return Graph::General; }

}; // class GenerateFunction

#endif // GRAPHWORKS_GENERATEFUNCTION_HPP_
