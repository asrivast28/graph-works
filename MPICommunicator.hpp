#ifndef GRAPHWORKS_MPICOMMUNICATOR_HPP_
#define GRAPHWORKS_MPICOMMUNICATOR_HPP_

#include <mpi.h>

class MPICommunicator {
public:
  MPICommunicator(MPI_Comm communicator = MPI_COMM_WORLD)
    : m_communicator(communicator)
  { 
    MPI_Comm_size(m_communicator, &m_size);
    MPI_Comm_rank(m_communicator, &m_rank);
  }

  MPICommunicator(const MPICommunicator& mpiCommunicator)
    : m_communicator(*mpiCommunicator),
    m_rank(mpiCommunicator.rank()),
    m_size(mpiCommunicator.size())
  {
  }

  MPI_Comm operator*() const { return m_communicator; }

  int size() const { return m_size; }

  int rank() const { return m_rank; }

private:
  const MPI_Comm m_communicator;	// the MPI communicator
  int m_rank;		// rank of the processor
  int m_size; 		// size of the communicator
}; // class MPICommunicator

#endif // GRAPHWORKS_MPICOMMUNICATOR_HPP_
