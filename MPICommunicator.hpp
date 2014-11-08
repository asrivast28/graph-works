#ifndef GRAPHWORKS_MPICOMMUNICATOR_HPP_
#define GRAPHWORKS_MPICOMMUNICATOR_HPP_

#include <mpi.h>

class MPICommunicator {
public:
  MPICommunicator(MPI_Comm communicator = MPI_COMM_WORLD)
    : m_communicator(communicator)
  { 
    int size, rank;
    MPI_Comm_size(m_communicator, &size);
    MPI_Comm_rank(m_communicator, &rank);

    m_size = static_cast<size_t>(size);
    m_rank = static_cast<size_t>(rank);
  }

  MPICommunicator(const MPICommunicator& mpiCommunicator)
    : m_communicator(*mpiCommunicator),
    m_rank(mpiCommunicator.rank()),
    m_size(mpiCommunicator.size())
  {
  }

  MPI_Comm operator*() const { return m_communicator; }

  size_t size() const { return m_size; }

  size_t rank() const { return m_rank; }

private:
  const MPI_Comm m_communicator;	// the MPI communicator
  size_t m_rank;		// rank of the processor
  size_t m_size; 		// size of the communicator
}; // class MPICommunicator

#endif // GRAPHWORKS_MPICOMMUNICATOR_HPP_
