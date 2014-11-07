/**
 * @file main.cpp
 * @brief
 * @version 1.0
 * @date 2014-11-03
 */

#include "GraphCompute.hpp"
#include "MPICommunicator.hpp"

#include <mpi.h>

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);

  MPICommunicator mpiCommunicator(MPI_COMM_WORLD);

  GraphCompute graphCompute(mpiCommunicator);

  MPI_Finalize();
  return 0;
}
