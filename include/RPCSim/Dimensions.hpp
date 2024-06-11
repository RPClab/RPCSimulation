#ifndef RPCSIM_DIMENSIONS_H_
#define RPCSIM_DIMENSIONS_H_

#include <array>

namespace RPCSim
{
  /**
   * @brief Dimensions Independent of coordinates. length is around X, width around Z and height around Y if I understood correctly Garfield++ jargon
   * 
   */
  class Dimensions
  {
    public:
    Dimensions()=default;
    Dimensions(const double& length, const double& width, const double& height) : m_dimensions({length,width,height}) {}
    double& operator[](const std::size_t& i) { return m_dimensions[i];}
    std::array<double,3> get() { return m_dimensions; }
    double length() { return m_dimensions[0];}
    double width() { return m_dimensions[1];}
    double heigth() { return m_dimensions[2];}
    private:
    std::array<double,3> m_dimensions;
  };


}

#endif