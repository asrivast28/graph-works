#ifndef GRAPHWORKS_GRAPH_HPP_
#define GRAPHWORKS_GRAPH_HPP_

#include <cstddef>
#include <vector>

class CombineFunction;

class MPIDataHelper;

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

  class Node {
    public:
      size_t
      index() const;

      bool
      isRoot() const;

      bool
      isLeaf() const;

      bool
      isParent(const Node&) const;

      bool
      isChild(const Node&) const;

      size_t
      numChildren() const;
  };

  typedef std::vector<Node>::iterator NodeIterator;

  typedef std::vector<Node>::const_iterator ConstNodeIterator;

  NodeIterator
  begin();

  ConstNodeIterator
  begin() const;

  NodeIterator
  end();

  ConstNodeIterator
  end() const;

  size_t
  size() const;

  template <AlgorithmChoice>
  bool
  compute(
    CombineFunction&,
    const MPIDataHelper&,
    const std::vector<std::vector<Node> >&
  );

  ~Graph();

private:
  std::vector<Node> m_nodeList;
}; // class Graph

#endif // GRAPHWORKS_GRAPH_HPP_
