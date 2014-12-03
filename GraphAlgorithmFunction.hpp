#ifndef GRAPHWORKS_ALGORITHMFUNCTION_HPP_
#define GRAPHWORKS_ALGORITHMFUNCTION_HPP_
#include <vector>
#include "Graph.hpp"

class GraphAlgorithmFunction {
private:
  const MPICommunicator& m_mpiCommunicator;
public:
  GraphAlgorithmFunction(const MPICommunicator& mpiCommunicator) : m_mpiCommunicator(mpiCommunicator){};

  virtual  bool  operator()(
    Graph::Graph&,
	const CombineFunction&,
	const std::vector<std::vector<Graph::Node> >&
  ) const = 0;
  virtual Graph::AlgorithmChoice getType();

};

#endif // GRAPHWORKS_ALGORITHMFUNCTION_HPP_
