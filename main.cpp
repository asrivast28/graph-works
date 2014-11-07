/**
 * @file main.cpp
 * @brief
 * @version 1.0
 * @date 2014-11-03
 */

#include "GraphCompute.hpp"

#include <mpi.h>

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);

  MPI_Finalize();
  return 0;
}
