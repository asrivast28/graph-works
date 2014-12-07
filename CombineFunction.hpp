#ifndef GRAPHWORKS_COMBINEFUNCTION_HPP_
#define GRAPHWORKS_COMBINEFUNCTION_HPP_

#include "Graph.hpp"

class CombineFunction {
public:
  /**Implement one of these**/

  bool
  operator()(
    Graph::Node& u,
    const Graph::Node& v
  ) const = 0{
	  return false;
  }
  bool
  operator()(
    DataPoint& p1,
    const DataPoint& p2
  ) const = 0{
  	return false;
  }
}; // class CombineFunction


}; // class CombineFunction
#endif // GRAPHWORKS_COMBINEFUNCTION_HPP_
