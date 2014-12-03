#include "GraphCompute.hpp"

#include "CombineFunction.hpp"
#include "GenerateFunction.hpp"
#include "MPICommunicator.hpp"

#include <iostream>

GraphCompute::GraphCompute(
  const MPICommunicator& mpiCommunicator
) : m_mpiCommunicator(mpiCommunicator)
{
}

/**
 * @brief General version of interaction set generator.
 *
 * @param g                 Graph on which computation is to be done.
 * @param generate          User provided generate function.
 * @param generateType      Type of generate function.
 * @param node              Node for which interaction set is to be generated.
 * @param interactionSets   Interaction sets for all the nodes of the graph.
 *
 * @return Dependency flag for the node.
 */
bool
GraphCompute::generateInteractionSetForNode(
  const Graph& g,
  const GenerateFunction& generate,
  const GraphAlgorithmChoice generateType,
  const GraphNode& node,
  std::vector<std::vector<GraphNode> >& interactionSets
) const
{
  std::vector<GraphNode> tempInteractionSet;
  std::back_insert_iterator<std::vector<GraphNode> > tempInteractionSetIter(tempInteractionSet);

  bool dependencyFlag = generate(g, node, tempInteractionSetIter);

  if (generateType == Graph::General) {
    interactionSets.push_back(tempInteractionSet);
  }

  return dependencyFlag;
}

/**
 * @brief Interaction set generator for all the nodes.
 *
 * @param g                 Graph on which computation is to be done.
 * @param generate          User provided generate function.
 * @param generateType      Type of generate function.
 * @param interactionSets   Interaction sets for all the nodes of the graph.
 *
 * @return Dependency flag for all the nodes.
 *
 * There are g.size() nodes in the local graph. Apply generate function to
 * each of them and obtain a list of graph nodes for each.
 */
bool
GraphCompute::generateAllInteractionSets(
  const Graph& g,
  const GenerateFunction& generate,
  GraphAlgorithmChoice& generateType,
  std::vector<std::vector<GraphNode> >& interactionSets
) const
{
  bool dependencyFlag = false;

  // Apply generate function on all nodes of the graph.
  for (GraphNodeIterator ni = g.begin(); ni != g.end(); ++ni) {

    bool nodeDependencyFlag = generateInteractionSetForNode(g, generate, generateType, *ni, interactionSets);

    // Check that flag for each call to generate returns the same thing.
    if (ni == g.begin()) {
      dependencyFlag = nodeDependencyFlag;
    }
    else if (nodeDependencyFlag != dependencyFlag) {
      throw std::runtime_error("Dependency flags are not consistent!");
    }
  }

  // Identify the local computation case.
  // Each node should only have itself in its interaction set.
  if (generateType == Graph::General) {
    unsigned int i = 0;
    for (GraphNodeIterator ni = g.begin(); ni != g.end(); ++ni, ++i) {
      // Check if interaction set size is 1 and if it contains only the node.
      if ((interactionSets[i].size() != 1) || (interactionSets[i][0].index() != (*ni).index())) {
        break;
      }
    }
    if (i == interactionSets.size()) {
      generateType = Graph::LocalComputation;
    }
  }

  MPI_Barrier(*m_mpiCommunicator);

  return dependencyFlag;
}

/**
 * @brief Function for detecting combine case. 
 *
 * @param g                 Graph on which computation is to be done.
 * @param generateType      Type of generate function.
 * @param interactionSets   Interaction sets for all the nodes of the graph.
 * @param dependencyFlag    Dependency flag for all the nodes.
 *
 * @return Deduced combine case based on generate type and dependency flag.
 */
GraphCompute::GraphAlgorithmChoice
GraphCompute::detectCombineCase(
  const Graph& g,
  const GraphAlgorithmChoice generateType,
  const std::vector<std::vector<GraphNode> >& interactionSets,
  const bool dependencyFlag
) const
{
  GraphAlgorithmChoice combineCase = Graph::General;
  if (dependencyFlag == false) {
    // This is simple, just use each node in the interaction set
    // of each local node and perform the computations.
    if (generateType == Graph::LocalComputation) {
      combineCase = Graph::LocalComputation;
    }
    else {
      combineCase = Graph::NoDependency;
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

    if (generateType == Graph::LocalComputation) {
      combineCase = Graph::LocalComputation;
    }
    else if (generateType == Graph::UpwardAccumulateSpecial) {
      combineCase = Graph::UpwardAccumulateSpecial;
    }
    else if (generateType == Graph::DownwardAccumulateSpecial) {
      combineCase = Graph::DownwardAccumulateSpecial;
    }
    else {
      // detect the cases for the special cases of upward and downward accumulations

      // check for special downward accumulation:
      if (combineCase == Graph::General) {
        //std::cout << "Checking for the special case of downward accumulation"
        //	<< std::endl;
        // for each node check if the I-set has only one node in it
        // and check about the levels of the nodes
        unsigned int i = 0;
        for (GraphNodeIterator ni = g.begin(); ni != g.end(); ++ni, ++i) {
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
          combineCase = Graph::DownwardAccumulateSpecial;
        }
      }

      // check for special upward accumulation:
      if (combineCase == Graph::General) {
        // for each node check if the I-set has same # of nodes as its # of children
        // and check for each node if it is its child
        unsigned int i = 0;
        for (GraphNodeIterator ni = g.begin(); ni != g.end(); ++ni, ++i) {
          if (!(*ni).isLeaf()) {
            // check if i-set sizes are == num of children
            if ((*ni).numChildren() != interactionSets[i].size()) break;
            // check if the node in i-set of each node is its parent
            bool breakFlag = false;
            for (unsigned int j = 0; j < interactionSets[i].size(); ++j) {
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
          combineCase = Graph::UpwardAccumulateSpecial;
        }
      }

      // implement algorithm detection for other general cases
      // ...

    }
  }

  MPI_Barrier(*m_mpiCommunicator);

  // find consensus combine case among all procs
  GraphAlgorithmChoice* consensus = new GraphAlgorithmChoice[m_mpiCommunicator.size()];
  MPI_Allgather(&combineCase, 1, MPI_INT, consensus, 1, MPI_INT, *m_mpiCommunicator);
  for (unsigned int i = 0; i < m_mpiCommunicator.size(); ++i) {
    if (consensus[i] != combineCase) {
      throw std::runtime_error("Error in obtaining consensus for computations!");
    }
  }

  return combineCase;
}

/**
 * @brief 
 *
 * @param g                 Graph on which computation is to be done.
 * @param combine           User provided combine function.
 * @param interactionSets   Interaction sets for all the nodes of the graph.
 * @param combineCase       The algorithm type to be used for combining.
 */
void
GraphCompute::combineAll(
  Graph& g,
  const CombineFunction& combine,
  const std::vector<std::vector<GraphNode> >& interactionSets,
  const GraphAlgorithmChoice combineCase
) const
{
	GraphAlgorithmFactory& factory = new GraphAlgorithmFactory(
			m_mpiCommunicator);
	GraphAlgorithmFunction algorithm = factory.getAlgorithm(g, combineCase);
	algorithm(g, combine, interactionSets);
	MPI_Barrier(*m_mpiCommunicator);
}

/**
 * @brief Function which is called by the user for performing computations. 
 *
 * @param g          Graph on which computation is to be done.
 * @param generate   User provided generate function.
 * @param combine    User provided combine function.
 *
 * @return true if computation was successful, else return false. 
 */
bool
GraphCompute::operator()(
  Graph& g,
  const GenerateFunction& generate,
  const CombineFunction& combine
)
{
  if (m_mpiCommunicator.rank() == 0) {
    std::cout << "+ performing Graph compute ... ";
  }

  MPI_Barrier(*m_mpiCommunicator);

  try {
    std::vector<std::vector<GraphNode> > interactionSets;

    double graphComputeTotalTime = MPI_Wtime();

    double generateTime = MPI_Wtime();
    GraphAlgorithmChoice generateType = generate.type();
    bool dependencyFlag = generateAllInteractionSets(g, generate, generateType, interactionSets);
    generateTime = MPI_Wtime() - generateTime;

    double detectionTime = MPI_Wtime();
    GraphAlgorithmChoice combineCase = detectCombineCase(g, generateType, interactionSets, dependencyFlag);
    detectionTime = MPI_Wtime() - detectionTime;

    // Currently only the special cases, UpwardAccumulateSpecial and DownwardAccumulateSpecial,
    // are implemented, were the nodes in the all interaction sets are all children, or the parent,
    // respectively. In these cases, new dependency forest is not constructed.
    double computeTime = MPI_Wtime();
    combineAll(g, combine, interactionSets, combineCase);
    computeTime = MPI_Wtime() - computeTime;

    graphComputeTotalTime = MPI_Wtime() - graphComputeTotalTime;

    if (m_mpiCommunicator.rank() == 0) {
      std::cout << "done: "
        << graphComputeTotalTime * 1000 << "ms"
        << " [g: " << generateTime * 1000 << "ms"
        << ", d: " << detectionTime * 1000 << "ms"
        << ", c: " << computeTime * 1000 << "ms]"
        << std::endl;
    }

  }
  catch (std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    std::cerr << "Aborting!" << std::endl;
    return false;
  }

  return true;
}

GraphCompute::~GraphCompute(
)
{
}
