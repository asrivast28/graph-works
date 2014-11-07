#ifndef GRAPHWORKS_MPIDATAHELPER_HPP_
#define GRAPHWORKS_MPIDATAHELPER_HPP_

#include <mpi.h>

class MPIDataHelper {
public:
  MPIDataHelper(MPI_Comm communicator = MPI_COMM_WORLD)
    : m_communicator(communicator)
  { 
    MPI_Comm_size(m_communicator, &m_size);
    MPI_Comm_rank(m_communicator, &m_rank);
  }

  MPIDataHelper(const MPIDataHelper& mpiDataHelper)
    : m_communicator(mpiDataHelper.communicator()),
    m_rank(mpiDataHelper.rank()),
    m_size(mpiDataHelper.size())
  {
  }

  MPI_Comm communicator() const { return m_communicator; }

  int size() const { return m_size; }

  int rank() const { return m_rank; }

private:
  MPI_Comm m_communicator;	// the MPI communicator
  int m_rank;		// rank of the processor
  int m_size; 		// size of the communicator
}; // class MPIDataHelper

#endif // GRAPHWORKS_MPIDATAHELPER_HPP_
