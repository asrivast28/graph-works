#include "GraphCompute.hpp"

#include "CombineFunction.hpp"
#include "GenerateFunction.hpp"
#include "MPICommunicator.hpp"

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
 * @param node              Node for which interaction set is to be generated.
 * @param interactionSets   Interaction sets for all the nodes of the graph.
 * @param generateType      Type of generate function used.
 *
 * @return Dependency flag for the node.
 */
bool
GraphCompute::generateInteractionSetForNode(
  const Graph& g,
  GenerateFunction& generate,
  const typename Graph::Node &node,
  std::vector<std::vector<typename Graph::Node> >& interactionSets,
  GraphAlgorithmChoice& generateType
) const
{

  typedef typename Graph::Node GraphNode;

  std::vector<GraphNode> tempInteractionSet;
  std::back_insert_iterator<std::vector<GraphNode> > tempInteractionSetIter(tempInteractionSet);

  bool dependencyFlag = generate(g, node, tempInteractionSetIter);
  interactionSets.push_back(tempInteractionSet);

  generateType = Graph::General;

  return dependencyFlag;
}

/**
 * @brief Interaction set generator for all the nodes.
 *
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
bool
GraphCompute::generateAllInteractionSets(
  const Graph& g,
  GenerateFunction& generate,
  std::vector<std::vector<typename Graph::Node> >& interactionSets,
  GraphAlgorithmChoice& generateType
) const
{
  typedef typename Graph::ConstNodeIterator NodeIterator;

  bool dependencyFlag = false;

  // Apply generate function on all nodes of the graph.
  for (NodeIterator ni = g.begin(); ni != g.end(); ++ni) {
    GraphAlgorithmChoice nodeGenerateType = Graph::General;

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
  if (generateType == Graph::General) {
    size_t i = 0;
    for (NodeIterator ni = g.begin(); ni != g.end(); ++ni, ++i) {
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

void
GraphCompute::detectCombineCase(
  const Graph& g,
  const std::vector<std::vector<typename Graph::Node> >& interactionSets,
  const GraphAlgorithmChoice generateType,
  const bool dependencyFlag,
  GraphAlgorithmChoice& combineCase
) const
{
  typedef typename Graph::ConstNodeIterator NodeIterator;

  if (dependencyFlag == false) {
    // This is simple, just use each node in the interaction
    // set of each local node and perform the computations. Call this case no_dep.
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
        size_t i = 0;
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
          combineCase = Graph::DownwardAccumulateSpecial;
        }
      }

      // check for special upward accumulation:
      if (combineCase == Graph::General) {
        // for each node check if the I-set has same # of nodes as its # of children
        // and check for each node if it is its child
        size_t i = 0;
        for (NodeIterator ni = g.begin(); ni != g.end(); ++ni, ++i) {
          if (!(*ni).isLeaf()) {
            // check if i-set sizes are == num of children
            if ((*ni).numChildren() != interactionSets[i].size()) break;
            // check if the node in i-set of each node is its parent
            bool breakFlag = false;
            for (size_t j = 0; j < interactionSets[i].size(); ++j) {
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
  for (int i = 0; i < m_mpiCommunicator.size(); ++i) {
    if (consensus[i] != combineCase) {
      throw std::runtime_error("Error in obtaining consensus for computations!");
    }
  }
}

void
GraphCompute::combineAll(
  Graph& g,
  CombineFunction& combine,
  const std::vector<std::vector<typename Graph::Node> >& interactionSets,
  const GraphAlgorithmChoice combineCase,
  double& computeTime
) const
{
  computeTime = MPI_Wtime();
  // XXX: Revisit later.
  // This is a stupid way of calling algorithm specific function
  // but can't figure out a better way of doing this as of now.
  switch (combineCase) {
    case Graph::General:
      g.compute<Graph::General>(combine, m_mpiCommunicator, interactionSets);
      break;
    case Graph::LocalComputation:
      g.compute<Graph::LocalComputation>(combine, m_mpiCommunicator, interactionSets);
      break;
    case Graph::NoDependency:
      g.compute<Graph::NoDependency>(combine, m_mpiCommunicator, interactionSets);
      break;
    case Graph::UpwardAccumulateReverse:
      g.compute<Graph::UpwardAccumulateReverse>(combine, m_mpiCommunicator, interactionSets);
      break;
    case Graph::UpwardAccumulateSpecial:
      g.compute<Graph::UpwardAccumulateSpecial>(combine, m_mpiCommunicator, interactionSets);
      break;
    case Graph::UpwardAccumulateGeneral:
      g.compute<Graph::UpwardAccumulateGeneral>(combine, m_mpiCommunicator, interactionSets);
      break;
    case Graph::DownwardAccumulateReverse:
      g.compute<Graph::DownwardAccumulateReverse>(combine, m_mpiCommunicator, interactionSets);
      break;
    case Graph::DownwardAccumulateSpecial:
      g.compute<Graph::DownwardAccumulateSpecial>(combine, m_mpiCommunicator, interactionSets);
      break;
    case Graph::DownwardAccumulateGeneral:
      g.compute<Graph::DownwardAccumulateGeneral>(combine, m_mpiCommunicator, interactionSets);
      break;
    default:
      throw std::runtime_error("Call for some algorithm choice is missing!");
  }
  computeTime = MPI_Wtime() - computeTime;

  MPI_Barrier(*m_mpiCommunicator);
}

bool
GraphCompute::operator()(
  Graph& g,
  GenerateFunction& generate,
  CombineFunction& combine
)
{
  if (m_mpiCommunicator.rank() == 0) {
    std::cout << "+ performing Graph compute ... ";
  }

  std::vector<std::vector<typename Graph::Node> > interactionSets;

  MPI_Barrier(*m_mpiCommunicator);
  double graphComputeTotalTime = MPI_Wtime();


  double generateTime = MPI_Wtime();
  GraphAlgorithmChoice generateType = Graph::General;
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
  GraphAlgorithmChoice combineCase = Graph::General;
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

  if (m_mpiCommunicator.rank() == 0) {
    std::cout << "done: "
      << graphComputeTotalTime * 1000 << "ms"
      << " [g: " << generateTime * 1000 << "ms"
      << ", d: " << detectionTime * 1000 << "ms"
      << ", c: " << computeTime * 1000 << "ms]"
      << std::endl;
  }

  return true;
}

GraphCompute::~GraphCompute(
)
{
}
