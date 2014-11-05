#ifndef GRAPHWORKS_BASEGRAPH_HPP_
#define GRAPHWORKS_BASEGRAPH_HPP_

#include "MPIDataHelper.hpp"

#include <vector>

class BaseGraph {
public:
  enum AlgorithmChoice {
    None,
    LocalComputation,
    NoDependency,
    UpwardAccumulateReverse,
    UpwardAccumulateSpecial,
    UpwardAccumulateGeneral,
    DownwardAccumulateSpecial,
    DownwardAccumulateGeneral,
    DownwardAccumulateReverse
  };

  class Node {
  };

  template <typename CombineFunction, AlgorithmChoice>
  bool
  compute(
    CombineFunction combine,
    const MPIDataHelper& mpiDataHelper
  )
  {
    throw std::runtime_error("Computation for algorithm choice hasn't been implemented!");
  }

  template <typename CombineFunction, LocalComputation>
  bool
  compute(
    CombineFunction combine,
    const MPIDataHelper& mpiDataHelper
  )
  {
    // Apply combine function on each node in the node list.
    for (std::vector<Node>::const_iterator node = m_nodeList.begin(); node != m_nodeList.end(); ++node) {
      combine(*node, *node);
    }

    return true;
  }

  template <typename CombineFunction, NoDependency>
  bool
  compute(
    CombineFunction combine,
    const MPIDataHelper& mpiDataHelper,
    const std::vector<std::vector<Node> >& interactionSets
  )
  {
    // For each node in the node list, apply combine for all the nodes in its interacton set.
    for (size_t i = 0; i < m_nodeList.size(); ++ i) {
      for (size_t j = 0; j < interactionSets[i].size(); ++ j) {
        combine(m_nodeList[i], interactionSets[i][j]);
      }
    }

    return true;
  }

  template <typename CombineFunction, UpwardAccumulateSpecial>
  bool
  compute(
    CombineFunction combine,
    const MPIDataHelper& mpiDataHelper
  )
  {
  }

  template <typename CombineFunction, DownwardAccumulateSpecial>
  bool
  compute(
    CombineFunction combine,
    const MPIDataHelper& mpiDataHelper
  )
  {
  }

private:
  std::vector<Node> m_nodeList;
};

#endif // GRAPHWORKS_BASEGRAPH_HPP_
