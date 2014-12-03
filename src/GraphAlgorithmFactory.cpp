/*
 * GraphAlgorithmFactory.cpp
 *
 *  Created on: Dec 1, 2014
 *      Author: samindaw
 */

#include "GraphAlgorithmFactory.hpp"

using namespace std;

void GraphAlgorithmFactory::registerAlgorithm(GraphAlgorithmFunction& algorithm) {
	algorithms.push_back(algorithm);
}

void GraphAlgorithmFactory::unregisterAlgorithm(GraphAlgorithmFunction& algorithm) {
	//TODO
}

GraphAlgorithmFunction& GraphAlgorithmFactory::getAlgorithm(
		const Graph& g, const Graph::AlgorithmChoice algorithmChoice) {
	for(int i=0;i<algorithms.size();i++){
		if (algorithms[i].getType()==algorithmChoice){
			return algorithms[i];
		}
	}
	GraphAlgorithmFunction *a;
	return *a;
}

