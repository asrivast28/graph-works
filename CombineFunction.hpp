#ifndef GRAPHWORKS_COMBINEFUNCTION_HPP_
#define GRAPHWORKS_COMBINEFUNCTION_HPP_

#include "Graph.hpp"

class DataPoint;

class CombineFunction {
public:
  /**Implement one of these**/

  virtual
  bool
  operator()(
    Graph::Node& u,
    const Graph::Node& v
  ) const = 0;

  virtual
  bool
  operator()(
    DataPoint& p1,
    const DataPoint& p2
  ) const = 0;
}; // class CombineFunction


#endif // GRAPHWORKS_COMBINEFUNCTION_HPP_
