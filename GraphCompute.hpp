#ifndef GRAPHWORKS_GRAPHCOMPUTE_HPP_
#define GRAPHWORKS_GRAPHCOMPUTE_HPP_

#include "BaseGraph.hpp"
#include "MPIDataHelper.hpp"

#include <iostream>
#include <vector>

class GraphCompute {
private:
  enum AlgorithmChoice {
    None,
    LocalComputation,
    NoDependency,
    UpwardAccumulationReverse,
    UpwardAccumulationSpecial,
    UpwardAccumulationGeneral,
    DownwardAccumulationSpecial,
    DownwardAccumulationGeneral,
    DownwardAccumulationReverse
  };

public:
  GraphCompute(MPI_Comm communicator) : m_mpiDataHelper(communicator) { }

  // the general version
  template <typename Graph, typename GenerateFunction>
  bool
  generateInteractionSet(
    const Graph& g,
    GenerateFunction generate,
    const typename Graph::Node &node,
    std::vector<std::vector<typename Graph::Node> >& interactionSets,
    AlgorithmChoice& generateType
  ) const
  {

    //std::cout << m_mpiDataHelper.rank() << ". This is the curious case of generality."
    //	<< std::endl;

    typedef typename Graph::Node GraphNode;

    std::vector<GraphNode> tempInteractionSet;
    std::back_insert_iterator<std::vector<GraphNode> > tempInteractionSetIter(tempInteractionSet);

    bool tempFlag = generate(g, node, tempInteractionSetIter);
    interactionSets.push_back(tempInteractionSet);
    
    generateType = None;

    // test
    //std::cout << m_mpiDataHelper.rank() << "." << " "
    //	<< tempInteractionSet.size() << std::endl;
    /*std::cout << m_mpiDataHelper.rank() << ". I-set of " << node.small_cell()
      << ", size = " << tempInteractionSet.size() << std::endl;
    for (int i = 0; i < tempInteractionSet.size(); ++ i) {
      std::cout << "  " << i << ". " << tempInteractionSet[i].is_leaf()
        << " " << tempInteractionSet[i].level()
        << " " << tempInteractionSet[i].small_cell()
        << " " << tempInteractionSet[i].large_cell()
        << " (" << tempInteractionSet[i].parent().proc_id_
        << ", " << tempInteractionSet[i].parent().index_
        << ") " << tempInteractionSet[i].num_children()
        << " " << tempInteractionSet[i].num_points()
        << std::endl;
    } */

    return tempFlag;
  }

  // there are g.size() nodes in the local graph
  // apply generate function to each of them
  // and obtain a list of graph nodes for each
  template <typename Graph, typename GenerateFunction>
  bool 
  generateAll(
    const Graph& g,
    GenerateFunction generate,
    std::vector<std::vector<typename Graph::Node> >& interactionSets,
    AlgorithmChoice& generateType
  ) const
  {
    typedef typename Graph::iterator NodeIterator;

    bool dependencyFlag = false;

    // apply generate function on all nodes of the Graph
    for (NodeIterator ni = g.begin(); ni != g.end(); ++ ni) {
      //unsigned long int memory = jaz::mem_usage();
      //if (m_mpiDataHelper.rank() == 0)
      //	 std::cout << m_mpiDataHelper.rank() << "." << temp << " Memory used in bytes = "
      //		 << memory << std::endl;

      //if (m_mpiDataHelper.rank() == 0)
      //std::cout << m_mpiDataHelper.rank() << ". [" << (*ni).index().proc_id_ << ", "
      //	<< (*ni).index().index_ << "], parent = ("
      //	<< (*ni).parent().proc_id_ << ", "
      //	<< (*ni).parent().index_ << ")" << std::endl;

      AlgorithmChoice tempGenerateType = None;
      bool tempFlag = generateInteractionSet(g, generate, *ni, interactionSets, tempGenerateType);

      // check that flag for each call to generate returns the same thing
      if (ni == g.begin()) {
        dependencyFlag = tempFlag;
        generateType = tempGenerateType;
      }
      else {
        if (tempFlag != dependencyFlag) {
          throw std::runtime_error("Grave error: dependency flag of all interaction sets are not the same! Aborting.");
        }
        if (tempGenerateType != generateType) {
          throw std::runtime_error("Grave error: Conflict in generateType! Aborting.");
        }
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
    AlgorithmChoice generateType,
    const bool dependencyFlag,
    AlgorithmChoice& combineCase
  ) const
  {
    typedef typename Graph::iterator NodeIterator;
    // identify the local computation case: each node has itself in its i-set
    // each node should have 1 node in its i-set and it should be itself
    // (the dependency flag may be either true or false)
    if (generateType == None) {
      int i = 0;
      for (NodeIterator ni = g.begin(); ni != g.end(); ++ ni, ++ i) {
        // check if i-set sizes are == 1
        if (interactionSets[i].size() != 1) {
          break;
        }
        // check if the node in i-set is itself
        if (!(interactionSets[i][0].index() == (*ni).index())) {
          break;
        }
      }
      if (i == interactionSets.size()) {
        generateType = LocalComputation;
      }
    }

    if (dependencyFlag == false) {
      // This is simple, just use each node in the interaction
      // set of each local node and perform the computations. Call this case no_dep.
      if (generateType == LocalComputation) {
        combineCase = LocalComputation;
      }
      else {
        combineCase = NoDependency;
      }
    }
    else {
      // if dependency flag is set, then using the interaction set of each local node,
      // find a local consensus of the levels, the special cases of children/parent only,
      // and conditions for uniqueness of parent, and then find a global consensus.
      // if there is no global consensus, notify that dependency cannot be satisfied.
      // Else, the following cases occur for each local node u and remote node v:
      // 	* UpwardAccumulationReverse. each node in at most 1 i-set, and level(u) > level(v) 
      // 	* b. each node in at most 1 i-set, and level(u) < level(v)
      // 	* c. each node has at most 1 in i-set, and level(u) > level(v)
      // 	* DownwardAccumulationReverse. each node has at most 1 in i-set, and level(u) < level(v)
      // Cases UpwardAccumulationReverse and b result in upward accumulation (where in
      // UpwardAccumulationReverse, the Graph is upside-down, and b is the original case of
      // upward Graph accumulation when all v are u's children).
      // Cases c and DownwardAccumulationReverse result in downward accumulation (where in
      // DownwardAccumulationReverse, the Graph is upside-down, and c is the original case of
      // downward Graph accumulation).
      // Case b -> either one of the following:
      // 	* UpwardAccumulationSpecial: where each all (and only) children are in the i-set.
      // 	* UpwardAccumulationGeneral: the other cases.
      // Case c -> either one of the following:
      // 	* DownwardAccumulationSpecial: where only the parent is present in i-set.
      // 	* DownwardAccumulationGeneral: the other cases.

      if (generateType == LocalComputation) {
        combineCase = LocalComputation;
      }
      else if (generateType == UpwardAccumulationSpecial) {
        combineCase = UpwardAccumulationSpecial;
      }
      else if (generateType == DownwardAccumulationSpecial) {
        combineCase = DownwardAccumulationSpecial;
      }
      else {
        // detect the cases for the special cases of upward and downward accumulations

        // check for special downward accumulation:
        if (combineCase == None) {
          //std::cout << "Checking for the special case of downward accumulation"
          //	<< std::endl;
          // for each node check if the I-set has only one node in it
          // and check about the levels of the nodes
          int i = 0;
          for (NodeIterator ni = g.begin(); ni != g.end(); ++ ni, ++ i) {
            if (!(*ni).is_root()) {
              // check if i-set sizes are == 1
              if (interactionSets[i].size() != 1) break;
              // check if the node in i-set of each node is its parent
              if (!interactionSets[i][0].is_parent(*ni)) break;
            }
          }
          if (i == interactionSets.size()) combineCase = DownwardAccumulationSpecial;
        }

        // check for special upward accumulation:
        if (combineCase == None) {
          //std::cout << "Checking for the special case of upward accumulation"
          //	<< std::endl;
          // for each node check if the I-set has same # of nodes as its # of children
          // and check for each node if it is its child
          int i = 0;
          for (NodeIterator ni = g.begin(); ni != g.end(); ++ ni, ++ i) {
            if (!(*ni).is_leaf()) {
              // check if i-set sizes are == num of children
              if ((*ni).num_children() != interactionSets[i].size()) break;
              // check if the node in i-set of each node is its parent
              bool breakFlag = false;
              for (int j = 0; j < interactionSets[i].size(); ++j) {
                if (!interactionSets[i][j].is_child(*ni)) {
                  breakFlag = true;
                  break;
                }
              }
              if (breakFlag) break;
            }
          }
          if (i == interactionSets.size()) combineCase = UpwardAccumulationSpecial;
        }

        // implement algorithm detection for other general cases
        // ...

      }
    }

    MPI_Barrier(m_mpiDataHelper.communicator());

    // find consensus combine case among all procs
    AlgorithmChoice* consensus = new AlgorithmChoice[m_mpiDataHelper.size()];
    MPI_Allgather(&combineCase, 1, MPI_INT, consensus, 1, MPI_INT, m_mpiDataHelper.communicator());
    for (int i = 0; i < m_mpiDataHelper.size(); ++ i) {
      if (consensus[i] != combineCase) {
        throw std::runtime_error("Error in obtaining consensus for computations! Aborting.");
      }
    }
  }

  template <typename Graph, typename CombineFunction>
  void
  combineAll(
    const Graph& g,
    CombineFunction combine,
    const std::vector<std::vector<typename Graph::Node> >& interactionSets,
    const AlgorithmChoice combineCase,
    double& computeTime
  ) const
  {
    switch (combineCase) {
      case LocalComputation:
        // Local computation: apply the combine function to each node locally
        //std::cout << "Local computations case." << std::endl;
        computeTime = MPI_Wtime();
        g.LocalCompute(combine, m_mpiDataHelper);
        computeTime = MPI_Wtime() - computeTime;
        break;

      case NoDependency:
        // look into the paper dealing with this case and do the corresponding
        // implementation for special cases if needed.
        // All the nodes in the interaction set are already available at the
        // local processor, since they were required from the generate function.
        //std::cout << m_mpiDataHelper.rank() << ". No dependency case." << std::endl;
        computeTime = MPI_Wtime();
        g.NoDependencyCompute(combine, interactionSets, m_mpiDataHelper);
        computeTime = MPI_Wtime() - computeTime;
        break;

      case UpwardAccumulationSpecial:
        // The Upward Accumulation: i-set has all and only the children, for all nodes
        //std::cout << "Upward accumulation special case." << std::endl;
        computeTime = MPI_Wtime();
        g.UpwardAccumulation(combine, m_mpiDataHelper);
        computeTime = MPI_Wtime() - computeTime;
        break;

      case DownwardAccumulationSpecial:
        // The Downward Accumulation: i-set has only one node, and it is the
        // parent, for all nodes
        //std::cout << "Downward accumulation special case." << std::endl;
        computeTime = MPI_Wtime();
        g.DownwardAccumulation(combine, m_mpiDataHelper);
        computeTime = MPI_Wtime() - computeTime;
        break;

      case UpwardAccumulationReverse:
      case UpwardAccumulationGeneral:
      case DownwardAccumulationGeneral:
      case DownwardAccumulationReverse:
        throw std::runtime_error("Not yet implemented!");

      default:
        throw std::runtime_error("Something went very wrong in algorithm detection.");
    } // switch

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

    MPI_Barrier(m_mpiDataHelper.communicator());

    double graphComputeTotalTime = MPI_Wtime();

    std::vector<std::vector<typename Graph::Node> > interactionSets;

    double generateTime = MPI_Wtime();

    AlgorithmChoice generateType = None;

    bool dependencyFlag;
    try {
      dependencyFlag = generateAll(g, generate, interactionSets, generateType);
    }
    catch (std::runtime_error& e) {
      std::cerr << e.what() << std::endl;
      return false;
    }

    generateTime = MPI_Wtime() - generateTime;

    double detectionTime = MPI_Wtime();

    AlgorithmChoice combineCase = None;

    try {
      detectCombineCase(g, interactionSets, generateType, dependencyFlag, combineCase);
    }
    catch (std::runtime_error& e) {
      std::cerr << e.what() << std::endl;
      return false;
    }

    detectionTime = MPI_Wtime() - detectionTime;

    // Currently only the special cases, UpwardAccumulationSpecial and DownwardAccumulationSpecial,
    // are implemented, were the nodes in the all i-sets are all children, or the parent,
    // respectively. In these cases, new dependency forest is not constructed.

    double computeTime = 0.0;

    try {
      combineAll(g, combine, interactionSets, combineCase, computeTime);
    }
    catch (std::runtime_error& e) {
      std::cerr << e.what() << std::endl;
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

    // test
    /*if (m_mpiDataHelper.rank() == 0)
      g.printGraph(); */

    return true;
  }

private:
  MPIDataHelper m_mpiDataHelper;
}; // class GraphCompute

#endif // GRAPHWORKS_GRAPHCOMPUTE_HPP_
