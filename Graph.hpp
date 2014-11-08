#ifndef GRAPHWORKS_GRAPH_HPP_
#define GRAPHWORKS_GRAPH_HPP_

#include "InputData.hpp"

#include <cstddef>
#include <vector>

class CombineFunction;
class MPICommunicator;

class Graph {
public:
  enum AlgorithmChoice {
    General,
    LocalComputation,
    NoDependency,
    UpwardAccumulateReverse,
    UpwardAccumulateSpecial,
    UpwardAccumulateGeneral,
    DownwardAccumulateSpecial,
    DownwardAccumulateGeneral,
    DownwardAccumulateReverse
  };

public:
  Graph(
    const InputData::Point* const,
    const unsigned int,
    const MPICommunicator&
  );

  class Node {
    public:
      typedef unsigned int IndexType;

    public:
      Node();

      Node(const Node&);

      IndexType
      index() const;

      bool
      isRoot() const;

      bool
      isLeaf() const;

      bool
      isParent(const Node&) const;

      bool
      isChild(const Node&) const;

      unsigned int 
      numChildren() const;

    private:
      IndexType m_index;
  }; // class Node

  typedef typename std::vector<Node>::iterator NodeIterator;

  typedef typename std::vector<Node>::const_iterator ConstNodeIterator;

  NodeIterator
  begin();

  ConstNodeIterator
  begin() const;

  NodeIterator
  end();

  ConstNodeIterator
  end() const;

  unsigned int 
  size() const;

  template <AlgorithmChoice>
  bool
  compute(
    const CombineFunction&,
    const std::vector<std::vector<Node> >&
  );

  ~Graph();

private:
  std::vector<Node> m_nodeList;
  const MPICommunicator& m_mpiCommunicator;
}; // class Graph

#endif // GRAPHWORKS_GRAPH_HPP_
