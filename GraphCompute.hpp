#ifndef GRAPHWORKS_GRAPHCOMPUTE_HPP_
#define GRAPHWORKS_GRAPHCOMPUTE_HPP_

#include "Graph.hpp"

#include <vector>

class CombineFunction;
class GenerateFunction;
class MPICommunicator;

class GraphCompute {
public:
  typedef typename Graph::AlgorithmChoice GraphAlgorithmChoice;
  typedef typename Graph::Node GraphNode;
  typedef typename Graph::ConstNodeIterator GraphNodeIterator;

public:
  GraphCompute(const MPICommunicator&);

  bool
  operator()(
    Graph&,
    GenerateFunction&,
    CombineFunction&
  );

  ~GraphCompute();

private:
  bool
  generateInteractionSetForNode(
    const Graph&,
    GenerateFunction&,
    const GraphAlgorithmChoice,
    const GraphNode& node,
    std::vector<std::vector<GraphNode> >&
  ) const;

  bool
  generateAllInteractionSets(
    const Graph&,
    GenerateFunction&,
    GraphAlgorithmChoice&,
    std::vector<std::vector<GraphNode> >&
  ) const;

  GraphAlgorithmChoice
  detectCombineCase(
    const Graph&,
    const GraphAlgorithmChoice,
    const std::vector<std::vector<GraphNode> >&,
    const bool
  ) const;

  void
  combineAll(
    Graph&,
    CombineFunction&,
    const std::vector<std::vector<GraphNode> >&,
    const GraphAlgorithmChoice,
    double&
  ) const;

private:
  const MPICommunicator& m_mpiCommunicator;
}; // class GraphCompute

#endif // GRAPHWORKS_GRAPHCOMPUTE_HPP_
