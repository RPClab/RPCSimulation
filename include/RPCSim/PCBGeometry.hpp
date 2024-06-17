#ifndef RPCSIM_PCBGEOMETRY_H_
#define RPCSIM_PCBGEOMETRY_H_

#include <cstdint>
#include "RPCSim/Dimensions.hpp"
#include "TGeoMatrix.h"

namespace RPCSim
{

class PCBGeometry
{
  public:
    enum class Type : std::uint8_t 
    {
      Unknown,
      Top,
      Bottom,
    };
    PCBGeometry();
    void setType(const Type& type) { m_type = type; }
    void setLength(const double& length=50) { m_dimensions[0]=length;}
    void setWidth(const double& width=50) { m_dimensions[1]=width;}
    void setHeight();
    void build();
    void translate(TGeoMatrix& trans) { m_translation=&trans; }
    RPCSim::Dimensions getDimensions() { return m_dimensions; }
    double groundPosition() { return m_ground_position; }
    double stripPosition() { return m_strip_position; }
  private:
    void buildTop();
    void buildBottom();
    RPCSim::Dimensions m_dimensions;
    TGeoMatrix* m_translation{nullptr};
    double m_strip_thickness{0.0035};
    double m_FR4_thickness_strip{0.04};
    double m_honeycomb_thickness{0.3};
    double m_FR4_thickness_ground{0.04};
    double m_ground_thickness{0.0035};
    double m_ground_position{0};
    double m_strip_position{0};
    Type m_type{Type::Unknown};
};

}

#endif