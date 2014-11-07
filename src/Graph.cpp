#include "Graph.hpp"

#include "CombineFunction.hpp"
#include "MPICommunicator.hpp"

size_t
Graph::Node::index(
) const
{
  return 0;
}

bool
Graph::Node::isRoot(
) const
{
  return false;
}

bool
Graph::Node::isLeaf(
) const
{
  return false;
}
bool
Graph::Node::isParent(
  const Node& n
) const
{
  return false;
}

bool
Graph::Node::isChild(
  const Node& n
) const
{
  return false;
}

size_t
Graph::Node::numChildren(
) const
{
  return 0;
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

size_t
Graph::size(
) const
{
  return m_nodeList.size();
}

template <Graph::AlgorithmChoice>
bool
Graph::compute(
  CombineFunction& combine,
  const MPICommunicator& mpiCommunicator,
  const std::vector<std::vector<Node> >& interactionSets
)
{
  throw std::runtime_error("Computation for algorithm choice hasn't been implemented!");
  return false;
}

template <>
bool
Graph::compute<Graph::NoDependency>(
  CombineFunction& combine,
  const MPICommunicator& mpiCommunicator,
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

template <>
bool
Graph::compute<Graph::LocalComputation>(
  CombineFunction& combine,
  const MPICommunicator& mpiCommunicator,
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
  CombineFunction& combine,
  const MPICommunicator& mpiCommunicator,
  const std::vector<std::vector<Node> >& interactionSets
)
{
  return false;
}

template <>
bool
Graph::compute<Graph::DownwardAccumulateSpecial>(
  CombineFunction& combine,
  const MPICommunicator& mpiCommunicator,
  const std::vector<std::vector<Node> >& interactionSets
)
{
  return false;
}

Graph::~Graph(
)
{
}

template bool Graph::compute<Graph::General>(CombineFunction&, const MPICommunicator&, const std::vector<std::vector<Node> >&);
template bool Graph::compute<Graph::UpwardAccumulateReverse>(CombineFunction&, const MPICommunicator&, const std::vector<std::vector<Node> >&);
template bool Graph::compute<Graph::UpwardAccumulateGeneral>(CombineFunction&, const MPICommunicator&, const std::vector<std::vector<Node> >&);
template bool Graph::compute<Graph::DownwardAccumulateReverse>(CombineFunction&, const MPICommunicator&, const std::vector<std::vector<Node> >&);
template bool Graph::compute<Graph::DownwardAccumulateGeneral>(CombineFunction&, const MPICommunicator&, const std::vector<std::vector<Node> >&);
