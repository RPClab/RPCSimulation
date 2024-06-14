#ifndef RPCSIM_RPCGEOMETRY_H_
#define RPCSIM_RPCGEOMETRY_H_

#include "RPCSim/GasMixture.hpp"
#include "RPCSim/Medium.hpp"
#include "RPCSim/Dimensions.hpp"
#include "RPCSim/PCBGeometry.hpp"
#include <string>

namespace Garfield
{
  class Geometry;
}

namespace RPCSim
{

class GasMixture;
class PCBGeometry;

class RPCGeometry
{
public:
  RPCGeometry();
  void putOnTop(PCBGeometry&);
  void putOnBottom(PCBGeometry&);
  void setLength(const double& length=5) { m_dimensions[0]=length;}
  void setWidth(const double& width=5) { m_dimensions[1]=width;}
  void fillGasGap(RPCSim::GasMixture* gas_mixture);
  void setElectrodeMaterial(const RPCSim::Medium& medium = RPCSim::Glass());
  void build();
  void draw(const std::string& options="ogl");
  double HVAnodePosition();
  double startGasGap();
  double endGasGap();
  double HVCathodePosition();
  RPCSim::Dimensions getDimensions() {return m_dimensions;}
  Garfield::Geometry* getGeometry();
  void closeGeometry(); //FIXME fix this
private:
  void setHeight();
  // Hardcoded value for now
  const double m_gasGapThickness{0.1}; //1mm
  const double m_electrodeThickness{0.11}; //1.1mm
  const double m_graphiteThickness{0.01}; //100um
  const double m_mylarThickness{0.01}; //100um
  RPCSim::Dimensions m_dimensions;
  RPCSim::GasMixture* m_gas_mixture{nullptr};
  RPCSim::Medium      m_electrode_medium{RPCSim::Glass()};
  // Position of the languettes for HV
  double m_HVAnodePosition{0};
  double m_HVCathodePosition{0};
  double m_startGasGap{0};
  double m_endGasGap{0};
};

}
#endif