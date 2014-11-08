/**
 * @file main.cpp
 * @brief
 * @version 1.0
 * @date 2014-11-03
 */

#include "GraphCompute.hpp"
#include "InputData.hpp"
#include "MPICommunicator.hpp"

#include <mpi.h>

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);

  MPICommunicator mpiCommunicator(MPI_COMM_WORLD);

  InputData inputData;

  if (!inputData.read(std::string(argv[1]), mpiCommunicator)) {
    std::cerr << "Encountered error while reading input data!" << std::endl;
    std::cerr << "Aborting." << std::endl;
    MPI_Finalize();
    return 1;
  }

  Graph myGraph(inputData.points(), inputData.numLocalPoints(), mpiCommunicator); 

  GraphCompute graphCompute(mpiCommunicator);

  // graphCompute();

  MPI_Finalize();

  return 0;
}
