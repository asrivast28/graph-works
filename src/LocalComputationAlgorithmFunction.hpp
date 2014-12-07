#ifndef GRAPHWORKS_LOCALALGORITHMFUNCTION_HPP_
#define GRAPHWORKS_LOCALALGORITHMFUNCTION_HPP_

#include "GraphAlgorithmFunction.hpp"


class LocalComputationAlgorithmFunction : public GraphAlgorithmFunction {
public:
  GraphAlgorithmFunction(
    const MPICommunicator& mpiCommunicator
  ) : m_mpiCommunicator(mpiCommunicator) { }

  bool
  operator()(
    Graph& g,
    const CombineFunction& combine,
    const std::vector<std::vector<Graph::Node> >& interactionSet
  ) const
  {
	  for(int i = 0; i < g.getProcessorNodeList().size(); ++ i) {
			combine(g.getProcessorNodeList()[i], g.getProcessorNodeList()[i]);
		} // for

		return true;
  }

  float
  getScore(
    Graph&
    const std::vector<std::vector<Graph::Node> >&
  ) const
  {
    return 0.0f;
  }

  Graph::AlgorithmChoice
  getType() { return Graph::LocalComputation; }

private:
  const MPICommunicator& m_mpiCommunicator;
}; // class LocalComputationAlgorithmFunction

#endif // GRAPHWORKS_LOCALALGORITHMFUNCTION_HPP_
