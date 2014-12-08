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
    const Graph&,
    const Graph::Node&,
    std::back_insert_iterator<std::vector<Graph::Node> >&,
    bool&
  ) const
  {
    return false;
  }

  virtual
  bool
  operator()(
    const Graph&,
    const DataPoint&,
    std::back_insert_iterator<std::vector<DataPoint> >&,
    bool&
  ) const
  {
    return false;
  }

  virtual
  Graph::AlgorithmChoice
  type() const { return Graph::General; }


  virtual
  ~GenerateFunction() = 0;
}; // class GenerateFunction

#endif // GRAPHWORKS_GENERATEFUNCTION_HPP_
