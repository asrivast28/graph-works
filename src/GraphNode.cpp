#include "Graph.hpp"

Graph::Node::Node(
) : m_index(-1)
{
}

Graph::Node::Node(
  const Node& node
) : m_index(node.index())
{
}

Graph::Node::IndexType
Graph::Node::index(
) const
{
  return m_index;
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

unsigned int
Graph::Node::numChildren(
) const
{
  return 0;
}
