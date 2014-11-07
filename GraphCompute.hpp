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
    const typename Graph::Node &node,
    std::vector<std::vector<typename Graph::Node> >&,
    GraphAlgorithmChoice&
  ) const;

  bool
  generateAllInteractionSets(
    const Graph&,
    GenerateFunction&,
    std::vector<std::vector<typename Graph::Node> >&,
    GraphAlgorithmChoice&
  ) const;

  void
  detectCombineCase(
    const Graph&,
    const std::vector<std::vector<typename Graph::Node> >&,
    const GraphAlgorithmChoice,
    const bool,
    GraphAlgorithmChoice&
  ) const;

  void
  combineAll(
    Graph&,
    CombineFunction&,
    const std::vector<std::vector<typename Graph::Node> >&,
    const GraphAlgorithmChoice,
    double&
  ) const;

private:
  const MPICommunicator& m_mpiCommunicator;
}; // class GraphCompute

#endif // GRAPHWORKS_GRAPHCOMPUTE_HPP_
