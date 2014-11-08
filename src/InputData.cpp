#include "InputData.hpp"

#include "MPICommunicator.hpp"

#include <algorithm>
#include <fstream>

InputData::InputData(
) : m_points(0),
  m_numGlobalPoints(0),
  m_numLocalPoints(0)
{
}

InputData::Point::Point(
) : m_x(0.0),
  m_y(0.0),
  m_z(0.0)
{
}

void
InputData::Point::set(
  const double x,
  const double y,
  const double z
)
{
  m_x = x;
  m_y = y;
  m_z = z;
}

double
InputData::Point::x(
) const
{
  return m_x;
}

double
InputData::Point::y(
) const
{
  return m_y;
}

double
InputData::Point::z(
) const
{
  return m_z;
}

bool
InputData::read(
  const std::string& fileName,
  const MPICommunicator& mpiCommunicator
)
{
  unsigned int myRank = mpiCommunicator.rank();
  unsigned int numProcs = mpiCommunicator.size();

  std::ifstream inputFile;

  if (myRank == 0) {
    inputFile.open(fileName);
    if (!inputFile) {
      return false;
    }

    inputFile >> m_numGlobalPoints;
  }

  MPI_Bcast(&m_numGlobalPoints, 1, MPI_UNSIGNED, 0, *mpiCommunicator);

  unsigned int avgPoints = (m_numGlobalPoints / numProcs) + (m_numLocalPoints % numProcs) ? 1 : 0;
  unsigned int myOffset = myRank * avgPoints;
  if (m_numGlobalPoints > myOffset) {
    m_numLocalPoints = std::min(avgPoints, m_numGlobalPoints - myOffset); 
  }

  if (!allocate()) {
    return false;
  }

  if (myRank == 0) {
    double* tmpBuffer[2];
    MPI_Request request[2];
    bool active[2] = {false, false};

    for (unsigned int i = 0; i < 2; ++i) {
      tmpBuffer[i] = new (std::nothrow) double[avgPoints * 3];
      if (tmpBuffer[i] == 0) {
        return false;
      }
    }
    for (unsigned int proc = 0; proc < numProcs; ++proc) {
      unsigned int procPoints = m_numLocalPoints;
      if (proc != myRank) {
        double* readBuffer = tmpBuffer[proc % 2];
        if (active[proc % 2]) {
          MPI_Wait(&request[proc % 2], MPI_STATUS_IGNORE);
        }
        active[proc % 2] = true;
        MPI_Status status;
        MPI_Recv(&procPoints, 1, MPI_UNSIGNED, proc, proc, *mpiCommunicator, &status);
        for (unsigned int i = 0; i < procPoints; ++i) {
          inputFile >> readBuffer[(i * 3)];
          inputFile >> readBuffer[(i * 3) + 1];
          inputFile >> readBuffer[(i * 3) + 2];
        }
        MPI_Isend(readBuffer, procPoints, MPI_DOUBLE, proc, 0, *mpiCommunicator, &request[proc % 2]);
      }
      else {
        double x, y, z;
        for (unsigned int i = 0; i < m_numLocalPoints; ++i) {
          inputFile >> x;
          inputFile >> y;
          inputFile >> z;
          m_points[i].set(x, y, z);
        }
      }
    }
    for (unsigned int i = 0; i < 2; ++i) {
      if (active[i]) {
        MPI_Status status;
        MPI_Wait(&request[i], &status);
      }
    }
    for (unsigned int i = 0; i < 2; ++i) {
      delete[] tmpBuffer[i];
    }
  }
  else {
    double* readBuffer = new double[m_numLocalPoints];
    MPI_Send(&m_numLocalPoints, 1, MPI_UNSIGNED, 0, myRank, *mpiCommunicator);
    MPI_Status status;
    MPI_Recv(readBuffer, m_numLocalPoints, MPI_DOUBLE, 0, 0, *mpiCommunicator, &status);
    for (unsigned int i = 0; i < m_numLocalPoints; ++i) {
      m_points[i].set(readBuffer[(i * 3)], readBuffer[(i * 3) + 1], readBuffer[(i * 3) + 2]);
    }
  }
}

const InputData::Point*
InputData::points(
) const
{
  return m_points;
}

unsigned int
InputData::numLocalPoints(
) const
{
  return m_numLocalPoints;
}

bool
InputData::allocate(
)
{
  m_points = new (std::nothrow) Point[m_numLocalPoints]; 
  return (m_points != 0);
}

void
InputData::deallocate(
)
{
  if (m_points != 0) {
    delete[] m_points;
    m_points = 0;
  }
}

InputData::~InputData(
)
{
  deallocate();
}
