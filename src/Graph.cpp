#include "Graph.hpp"

#include "CombineFunction.hpp"
#include "MPICommunicator.hpp"

Graph::Graph(
  const InputData::Point* const points,
  const unsigned int numPoints,
  const MPICommunicator& mpiCommunicator
) : m_nodeList(),
  m_mpiCommunicator(mpiCommunicator)
{
}

Graph::NodeIterator
Graph::begin(
)
{
  return m_nodeList.begin();
}

Graph::ConstNodeIterator
Graph::begin(
) const
{
  return m_nodeList.begin();
}

Graph::NodeIterator
Graph::end(
)
{
  return m_nodeList.end();
}

Graph::ConstNodeIterator
Graph::end(
) const
{
  return m_nodeList.end();
}

unsigned int
Graph::size(
) const
{
  return static_cast<unsigned int>(m_nodeList.size());
}

template <Graph::AlgorithmChoice>
bool
Graph::compute(
  const CombineFunction& combine,
  const std::vector<std::vector<Node> >& interactionSets
)
{
  throw std::runtime_error("Computation for algorithm choice hasn't been implemented!");
  return false;
}

template <>
bool
Graph::compute<Graph::NoDependency>(
  const CombineFunction& combine,
  const std::vector<std::vector<Node> >& interactionSets
)
{
  // For each node in the node list, apply combine for all the nodes in its interacton set.
  for (unsigned int i = 0; i < m_nodeList.size(); ++ i) {
    for (unsigned int j = 0; j < interactionSets[i].size(); ++ j) {
      combine(m_nodeList[i], interactionSets[i][j]);
    }
  }

  return true;
}

template <>
bool
Graph::compute<Graph::LocalComputation>(
  const CombineFunction& combine,
  const std::vector<std::vector<Node> >& interactionSets
)
{
  // Apply combine function on each node in the node list.
  for (std::vector<Node>::iterator node = m_nodeList.begin(); node != m_nodeList.end(); ++node) {
    combine(*node, *node);
  }

  return true;
}

template <>
bool
Graph::compute<Graph::UpwardAccumulateSpecial>(
  const CombineFunction& combine,
  const std::vector<std::vector<Node> >& interactionSets
)
{
  return false;
}

template <>
bool
Graph::compute<Graph::DownwardAccumulateSpecial>(
  const CombineFunction& combine,
  const std::vector<std::vector<Node> >& interactionSets
)
{
  return false;
}

Graph::~Graph(
)
{
}


template bool Graph::compute<Graph::General>(const CombineFunction&, const std::vector<std::vector<Node> >&);
template bool Graph::compute<Graph::UpwardAccumulateReverse>(const CombineFunction&, const std::vector<std::vector<Node> >&);
template bool Graph::compute<Graph::UpwardAccumulateGeneral>(const CombineFunction&, const std::vector<std::vector<Node> >&);
template bool Graph::compute<Graph::DownwardAccumulateReverse>(const CombineFunction&, const std::vector<std::vector<Node> >&);
template bool Graph::compute<Graph::DownwardAccumulateGeneral>(const CombineFunction&, const std::vector<std::vector<Node> >&);
