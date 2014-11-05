#ifndef GRAPHWORKS_GRAPHCOMPUTE_HPP_
#define GRAPHWORKS_GRAPHCOMPUTE_HPP_

#include "BaseGraph.hpp"
#include "MPIDataHelper.hpp"

#include <iostream>
#include <vector>

class GraphCompute {
public:
  GraphCompute(MPI_Comm communicator) : m_mpiDataHelper(communicator) { }

  /**
   * @brief General version of interaction set generator.
   *
   * @tparam Graph            Type of graph.
   * @tparam GenerateFunction Type of generate function.
   * @param g                 Graph on which computation is to be done.
   * @param generate          User provided generate function.
   * @param node              Node for which interaction set is to be generated.
   * @param interactionSets   Interaction sets for all the nodes of the graph.
   * @param generateType      Type of generate function used.
   *
   * @return Dependency flag for the node.
   */
  template <typename Graph, typename GenerateFunction>
  bool
  generateInteractionSetForNode(
    const Graph& g,
    GenerateFunction generate,
    const typename Graph::Node &node,
    std::vector<std::vector<typename Graph::Node> >& interactionSets,
    BaseGraph::AlgorithmChoice& generateType
  ) const
  {

    typedef typename Graph::Node GraphNode;

    std::vector<GraphNode> tempInteractionSet;
    std::back_insert_iterator<std::vector<GraphNode> > tempInteractionSetIter(tempInteractionSet);

    bool dependencyFlag = generate(g, node, tempInteractionSetIter);
    interactionSets.push_back(tempInteractionSet);

    generateType = BaseGraph::None;

    return dependencyFlag;
  }

  /**
   * @brief Interaction set generator for all the nodes.
   *
   * @tparam Graph            Type of graph.
   * @tparam GenerateFunction Type of generate function.
   * @param g                 Graph on which computation is to be done.
   * @param generate          User provided generate function.
   * @param interactionSets   Interaction sets for all the nodes of the graph.
   * @param generateType      Type of generate function.
   *
   * @return Dependency flag for all the nodes.
   *
   * There are g.size() nodes in the local graph. Apply generate function to
   * each of them and obtain a list of graph nodes for each.
   */
  template <typename Graph, typename GenerateFunction>
  bool
  generateAllInteractionSets(
    const Graph& g,
    GenerateFunction generate,
    std::vector<std::vector<typename Graph::Node> >& interactionSets,
    BaseGraph::AlgorithmChoice& generateType
  ) const
  {
    typedef typename Graph::iterator NodeIterator;

    bool dependencyFlag = false;

    // Apply generate function on all nodes of the graph.
    for (NodeIterator ni = g.begin(); ni != g.end(); ++ni) {
      BaseGraph::AlgorithmChoice nodeGenerateType = BaseGraph::None;

      bool nodeDependencyFlag = generateInteractionSetForNode(g, generate, *ni, interactionSets, nodeGenerateType);

      // Check that flag for each call to generate returns the same thing.
      if (ni == g.begin()) {
        dependencyFlag = nodeDependencyFlag;
        generateType = nodeGenerateType;
      }
      else {
        if (nodeDependencyFlag != dependencyFlag) {
          throw std::runtime_error("Dependency flags are not consistent!");
        }
        if (nodeGenerateType != generateType) {
          throw std::runtime_error("Generate types are not consistent!");
        }
      }
    }

    // Identify the local computation case.
    // Each node should only have itself in its interaction set.
    if (generateType == BaseGraph::None) {
      int i = 0;
      for (NodeIterator ni = g.begin(); ni != g.end(); ++ni, ++i) {
        // Check if interaction set size is 1 and if it contains only the node.
        if ((interactionSets[i].size() != 1) || (interactionSets[i][0].index() != (*ni).index())) {
          break;
        }
      }
      if (i == interactionSets.size()) {
        generateType = BaseGraph::LocalComputation;
      }
    }

    MPI_Barrier(m_mpiDataHelper.communicator());

    return dependencyFlag;
  }

  template <typename Graph>
  void
  detectCombineCase(
    const Graph& g,
    const std::vector<std::vector<typename Graph::Node> >& interactionSets,
    const BaseGraph::AlgorithmChoice generateType,
    const bool dependencyFlag,
    BaseGraph::AlgorithmChoice& combineCase
  ) const
  {
    typedef typename Graph::iterator NodeIterator;

    if (dependencyFlag == false) {
      // This is simple, just use each node in the interaction
      // set of each local node and perform the computations. Call this case no_dep.
      if (generateType == BaseGraph::LocalComputation) {
        combineCase = BaseGraph::LocalComputation;
      }
      else {
        combineCase = BaseGraph::NoDependency;
      }
    }
    else {
      // if dependency flag is set, then using the interaction set of each local node,
      // find a local consensus of the levels, the special cases of children/parent only,
      // and conditions for uniqueness of parent, and then find a global consensus.
      // if there is no global consensus, notify that dependency cannot be satisfied.
      // Else, the following cases occur for each local node u and remote node v:
      // 	* UpwardAccumulateReverse. each node in at most 1 i-set, and level(u) > level(v)
      // 	* b. each node in at most 1 i-set, and level(u) < level(v)
      // 	* c. each node has at most 1 in i-set, and level(u) > level(v)
      // 	* DownwardAccumulateReverse. each node has at most 1 in i-set, and level(u) < level(v)
      // Cases UpwardAccumulateReverse and b result in upward accumulation (where in
      // UpwardAccumulateReverse, the Graph is upside-down, and b is the original case of
      // upward Graph accumulation when all v are u's children).
      // Cases c and DownwardAccumulateReverse result in downward accumulation (where in
      // DownwardAccumulateReverse, the Graph is upside-down, and c is the original case of
      // downward Graph accumulation).
      // Case b -> either one of the following:
      // 	* UpwardAccumulateSpecial: where each all (and only) children are in the i-set.
      // 	* UpwardAccumulateGeneral: the other cases.
      // Case c -> either one of the following:
      // 	* DownwardAccumulateSpecial: where only the parent is present in i-set.
      // 	* DownwardAccumulateGeneral: the other cases.

      if (generateType == BaseGraph::LocalComputation) {
        combineCase = BaseGraph::LocalComputation;
      }
      else if (generateType == BaseGraph::UpwardAccumulateSpecial) {
        combineCase = BaseGraph::UpwardAccumulateSpecial;
      }
      else if (generateType == BaseGraph::DownwardAccumulateSpecial) {
        combineCase = BaseGraph::DownwardAccumulateSpecial;
      }
      else {
        // detect the cases for the special cases of upward and downward accumulations

        // check for special downward accumulation:
        if (combineCase == BaseGraph::None) {
          //std::cout << "Checking for the special case of downward accumulation"
          //	<< std::endl;
          // for each node check if the I-set has only one node in it
          // and check about the levels of the nodes
          int i = 0;
          for (NodeIterator ni = g.begin(); ni != g.end(); ++ni, ++i) {
            if (!(*ni).isRoot()) {
              // check if i-set sizes are == 1
              if (interactionSets[i].size() != 1) {
                break;
              }
              // check if the node in i-set of each node is its parent
              if (!interactionSets[i][0].isParent(*ni)) {
                break;
              }
            }
          }
          if (i == interactionSets.size()) {
            combineCase = BaseGraph::DownwardAccumulateSpecial;
          }
        }

        // check for special upward accumulation:
        if (combineCase == BaseGraph::None) {
          // for each node check if the I-set has same # of nodes as its # of children
          // and check for each node if it is its child
          int i = 0;
          for (NodeIterator ni = g.begin(); ni != g.end(); ++ni, ++i) {
            if (!(*ni).isLeaf()) {
              // check if i-set sizes are == num of children
              if ((*ni).num_children() != interactionSets[i].size()) break;
              // check if the node in i-set of each node is its parent
              bool breakFlag = false;
              for (int j = 0; j < interactionSets[i].size(); ++j) {
                if (!interactionSets[i][j].isChild(*ni)) {
                  breakFlag = true;
                  break;
                }
              }
              if (breakFlag) {
                break;
              }
            }
          }
          if (i == interactionSets.size()) {
            combineCase = BaseGraph::UpwardAccumulateSpecial;
          }
        }

        // implement algorithm detection for other general cases
        // ...

      }
    }

    MPI_Barrier(m_mpiDataHelper.communicator());

    // find consensus combine case among all procs
    BaseGraph::AlgorithmChoice* consensus = new BaseGraph::AlgorithmChoice[m_mpiDataHelper.size()];
    MPI_Allgather(&combineCase, 1, MPI_INT, consensus, 1, MPI_INT, m_mpiDataHelper.communicator());
    for (int i = 0; i < m_mpiDataHelper.size(); ++i) {
      if (consensus[i] != combineCase) {
        throw std::runtime_error("Error in obtaining consensus for computations!");
      }
    }
  }

  template <typename Graph, typename CombineFunction>
  void
  combineAll(
    const Graph& g,
    CombineFunction combine,
    const std::vector<std::vector<typename Graph::Node> >& interactionSets,
    const BaseGraph::AlgorithmChoice combineCase,
    double& computeTime
  ) const
  {
    computeTime = MPI_Wtime();
    if (combineCase == BaseGraph::NoDependency) {
      g.template compute<CombineFunction, combineCase>(combine, m_mpiDataHelper, interactionSets);
    }
    else {
      g.template compute<CombineFunction, combineCase>(combine, m_mpiDataHelper);
    }
    computeTime = MPI_Wtime() - computeTime;

    MPI_Barrier(m_mpiDataHelper.communicator());
  }

  template <typename Graph, typename GenerateFunction, typename CombineFunction>
  bool
  operator()(
    Graph& g,
    GenerateFunction generate,
    CombineFunction combine
  )
  {
    if (m_mpiDataHelper.rank() == 0) {
      std::cout << "+ performing Graph compute ... ";
    }

    std::vector<std::vector<typename Graph::Node> > interactionSets;

    MPI_Barrier(m_mpiDataHelper.communicator());
    double graphComputeTotalTime = MPI_Wtime();


    double generateTime = MPI_Wtime();
    BaseGraph::AlgorithmChoice generateType = BaseGraph::None;
    bool dependencyFlag;
    try {
      dependencyFlag = generateAllInteractionSets(g, generate, interactionSets, generateType);
    }
    catch (std::runtime_error& e) {
      std::cerr << e.what() << std::endl;
      std::cerr << "Aborting!" << std::endl;
      return false;
    }
    generateTime = MPI_Wtime() - generateTime;

    double detectionTime = MPI_Wtime();
    BaseGraph::AlgorithmChoice combineCase = BaseGraph::None;
    try {
      detectCombineCase(g, interactionSets, generateType, dependencyFlag, combineCase);
    }
    catch (std::runtime_error& e) {
      std::cerr << e.what() << std::endl;
      std::cerr << "Aborting!" << std::endl;
      return false;
    }
    detectionTime = MPI_Wtime() - detectionTime;

    // Currently only the special cases, UpwardAccumulateSpecial and DownwardAccumulateSpecial,
    // are implemented, were the nodes in the all interaction sets are all children, or the parent,
    // respectively. In these cases, new dependency forest is not constructed.
    double computeTime = 0.0;
    try {
      combineAll(g, combine, interactionSets, combineCase, computeTime);
    }
    catch (std::runtime_error& e) {
      std::cerr << e.what() << std::endl;
      std::cerr << "Aborting!" << std::endl;
      return false;
    }

    graphComputeTotalTime = MPI_Wtime() - graphComputeTotalTime;

    if (m_mpiDataHelper.rank() == 0) {
      std::cout << "done: "
        << graphComputeTotalTime * 1000 << "ms"
        << " [g: " << generateTime * 1000 << "ms"
        << ", d: " << detectionTime * 1000 << "ms"
        << ", c: " << computeTime * 1000 << "ms]"
        << std::endl;
    }

    return true;
  }

private:
  MPIDataHelper m_mpiDataHelper;
}; // class GraphCompute

#endif // GRAPHWORKS_GRAPHCOMPUTE_HPP_
