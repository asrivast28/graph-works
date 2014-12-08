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
    Graph::Node&,
    const Graph::Node&
  ) const
  {
    return false;
  }

  virtual
  bool
  operator()(
    DataPoint&,
    const DataPoint&
  ) const
  {
    return false;
  }

  virtual
  ~CombineFunction() = 0;
}; // class CombineFunction

#endif // GRAPHWORKS_COMBINEFUNCTION_HPP_
