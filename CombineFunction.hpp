#ifndef GRAPHWORKS_COMBINEFUNCTION_HPP_
#define GRAPHWORKS_COMBINEFUNCTION_HPP_

#include "Graph.hpp"

class CombineFunction {
public:
  virtual
  bool
  operator()(
    Graph::Node&,
    const Graph::Node&
  ) const = 0;

}; // class CombineFunction

#endif // GRAPHWORKS_COMBINEFUNCTION_HPP_
