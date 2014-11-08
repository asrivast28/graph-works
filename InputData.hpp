#ifndef GRAPHWORKS_INPUTDATA_HPP_
#define GRAPHWORKS_INPUTDATA_HPP_

#include <string>

class MPICommunicator;

class InputData {
public:
  InputData();

  class Point {
    public:
      Point();

      void
      set(
        const double,
        const double,
        const double
      );

      double
      x() const;

      double
      y() const;

      double
      z() const;

    private:
      double m_x;
      double m_y;
      double m_z;
  }; // class Point

  bool
  read(
    const std::string&,
    const MPICommunicator&
  );

  const Point*
  points() const;

  unsigned int
  numLocalPoints() const;

  ~InputData();

private:
  bool 
  allocate();

  void
  deallocate();

private:
  Point* m_points;
  unsigned int m_numGlobalPoints;
  unsigned int m_numLocalPoints;
};

#endif // GRAPHWORKS_INPUTDATA_HPP_
