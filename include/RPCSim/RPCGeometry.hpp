#ifndef RPCSIM_RPCGEOMETRY_H_
#define RPCSIM_RPCGEOMETRY_H_

#include "Medium.hpp"
#include "RPCSim/GasMixture.hpp"
#include "RPCSim/Medium.hpp"
#include <string>

namespace Garfield
{
  class Geometry;
}

namespace RPCSim
{

class GasMixture;

class RPCGeometry
{
public:
  RPCGeometry();
  void fillGasGap(RPCSim::GasMixture* gas_mixture);
  void setElectrodeMaterial(const RPCSim::Medium& medium = RPCSim::Glass());
  void build();
  void draw(const std::string& options="ogl");
  Garfield::Geometry* getGeometry();
private:
  // Hardcoded value for now
  const double m_gasGapThickness{0.1}; //1mm
  const double m_electrodeThickness{0.11}; //1.1mm
  const double m_length{50}; //50cm
  const double m_width{50}; //50cm
  RPCSim::GasMixture* m_gas_mixture{nullptr};
  RPCSim::Medium      m_electrode_medium{RPCSim::Glass()};
};

}
#endif