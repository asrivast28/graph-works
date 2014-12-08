#ifndef GRAPHWORKS_ALGORITHMFACTORY_HPP_
#define GRAPHWORKS_ALGORITHMFACTORY_HPP_

#include "Graph.hpp"
#include "GraphAlgorithmFunction.hpp"

#include <vector>

class CombineFunction;
class GenerateFunction;
class MPICommunicator;

class GraphAlgorithmFactory {

public:
  GraphAlgorithmFactory(
    const MPICommunicator& mpiCommunicator
  ) : m_mpiCommunicator(mpiCommunicator)
  { }

  void
  registerAlgorithm(
    GraphAlgorithmFunction*
  );

  void
  unregisterAlgorithm(
    GraphAlgorithmFunction*
  );

  GraphAlgorithmFunction*
  getAlgorithm(
    const Graph&,
    const Graph::AlgorithmChoice
  );

private:
  const MPICommunicator& m_mpiCommunicator;
  std::vector<GraphAlgorithmFunction*> m_algorithms;
};

#endif // GRAPHWORKS_ALGORITHMFACTORY_HPP_
