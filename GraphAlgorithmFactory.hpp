#ifndef GRAPHWORKS_ALGORYTHMFACTORY_HPP_
#define GRAPHWORKS_ALGORYTHMFACTORY_HPP_

#include <vector>
#include "Graph.hpp"
#include "GraphAlgorithmFunction.hpp"

class CombineFunction;
class GenerateFunction;
class MPICommunicator;

class GraphAlgorithmFactory {

public:
  GraphAlgorithmFactory(const MPICommunicator& mpiCommunicator) : m_mpiCommunicator(mpiCommunicator){
  }
  void registerAlgorithm(GraphAlgorithmFunction& algorithm);
  void unregisterAlgorithm(GraphAlgorithmFunction& algorithm);
  GraphAlgorithmFunction& getAlgorithm(const Graph&, const Graph::AlgorithmChoice);

private:
  const MPICommunicator& m_mpiCommunicator;
private:
  std::vector<GraphAlgorithmFunction> algorithms;
};

#endif // GRAPHWORKS_ALGORYTHMFACTORY_HPP_
