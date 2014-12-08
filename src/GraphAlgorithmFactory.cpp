/*
 * GraphAlgorithmFactory.cpp
 *
 *  Created on: Dec 1, 2014
 *      Author: samindaw
 */

#include "GraphAlgorithmFactory.hpp"

void
GraphAlgorithmFactory::registerAlgorithm(
  GraphAlgorithmFunction* algorithm
)
{
	m_algorithms.push_back(algorithm);
}

void
GraphAlgorithmFactory::unregisterAlgorithm(
  GraphAlgorithmFunction*
)
{
	//TODO
}

GraphAlgorithmFunction*
GraphAlgorithmFactory::getAlgorithm(
  const Graph&,
  const Graph::AlgorithmChoice algorithmChoice
)
{
	for (GraphAlgorithmFunction* algorithm : m_algorithms) {
		if (algorithm->getType() == algorithmChoice) {
			return algorithm;
		}
	}
  return nullptr;
}

