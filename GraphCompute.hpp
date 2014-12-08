#ifndef GRAPHWORKS_GRAPHCOMPUTE_HPP_
#define GRAPHWORKS_GRAPHCOMPUTE_HPP_

#include "GenerateFunction.hpp"
#include "Graph.hpp"

#include <vector>

class CombineFunction;
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
    const GenerateFunction&,
    const CombineFunction&
  );

  ~GraphCompute();

private:
  bool
  generateInteractionSetForNode(
    const Graph&,
    const GenerateFunction&,
    const GraphAlgorithmChoice,
    const GraphNode& node,
    std::vector<std::vector<GraphNode> >&
  ) const;

  bool
  generateAllInteractionSets(
    const Graph&,
    const GenerateFunction&,
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
    const CombineFunction&,
    const std::vector<std::vector<GraphNode> >&,
    const GraphAlgorithmChoice
  ) const;

private:
  const MPICommunicator& m_mpiCommunicator;
}; // class GraphCompute

#endif // GRAPHWORKS_GRAPHCOMPUTE_HPP_
