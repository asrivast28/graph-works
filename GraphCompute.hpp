#ifndef GRAPHWORKS_GRAPHCOMPUTE_HPP_
#define GRAPHWORKS_GRAPHCOMPUTE_HPP_

#include "Graph.hpp"

#include <iostream>
#include <vector>

class CombineFunction;
class GenerateFunction;
class MPICommunicator;

class GraphCompute {
public:
  typedef typename Graph::AlgorithmChoice GraphAlgorithmChoice;

  GraphCompute(const MPICommunicator&);

  bool
  operator()(
    Graph& g,
    GenerateFunction& generate,
    CombineFunction& combine
  );

  ~GraphCompute();

private:
  bool
  generateInteractionSetForNode(
    const Graph& g,
    GenerateFunction& generate,
    const typename Graph::Node &node,
    std::vector<std::vector<typename Graph::Node> >& interactionSets,
    GraphAlgorithmChoice& generateType
  ) const;

  bool
  generateAllInteractionSets(
    const Graph& g,
    GenerateFunction& generate,
    std::vector<std::vector<typename Graph::Node> >& interactionSets,
    GraphAlgorithmChoice& generateType
  ) const;

  void
  detectCombineCase(
    const Graph& g,
    const std::vector<std::vector<typename Graph::Node> >& interactionSets,
    const GraphAlgorithmChoice generateType,
    const bool dependencyFlag,
    GraphAlgorithmChoice& combineCase
  ) const;

  void
  combineAll(
    Graph& g,
    CombineFunction& combine,
    const std::vector<std::vector<typename Graph::Node> >& interactionSets,
    const GraphAlgorithmChoice combineCase,
    double& computeTime
  ) const;

private:
  const MPICommunicator& m_mpiCommunicator;
}; // class GraphCompute

#endif // GRAPHWORKS_GRAPHCOMPUTE_HPP_
