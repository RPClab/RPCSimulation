#ifndef RPCSIM_PCBGEOMETRY_H_
#define RPCSIM_PCBGEOMETRY_H_

#include "RPCSim/Dimensions.hpp"
#include "TGeoMatrix.h"

namespace RPCSim
{

class PCBGeometry
{
  public:
    PCBGeometry();
    void setLength(const double& length=50) { m_dimensions[0]=length;}
    void setWidth(const double& width=50) { m_dimensions[1]=width;}
    void setHeight();
    void build();
    void translate(const TGeoTranslation& trans) { m_translation=trans; }
    RPCSim::Dimensions getDimensions() { return m_dimensions; }
  private:
    RPCSim::Dimensions m_dimensions;
    TGeoTranslation m_translation;
    double m_strip_thickness{0.0035};
    double m_FR4_thickness_strip{0.04};
    double m_honeycomb_thickness{0.3};
    double m_FR4_thickness_ground{0.04};
    double m_ground_thickness{0.0035};
};

}

#endif