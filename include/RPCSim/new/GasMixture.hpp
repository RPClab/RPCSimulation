#pragma once

#include<string>
#include<vector>

#include "Garfield/MediumMagboltz.hh"

namespace RPCSim
{
  class GasMixture
  {
  public:
    GasMixture()
    {

    };
    GasMixture(const std::string& filename);
    GasMixture(const std::vector<std::string>& gases, const std::vector<double>& percent);
    void setFile(const std::string& filename);
    void setMixture(const std::vector<std::string>& names, const std::vector<double>& percent);
    void initialise(const bool& init) { m_magboltz.Initialise(init);}
    void setTemperature(const double& temperature) { m_magboltz.SetTemperature(temperature+273.15);}
    void setPressure(const double& pressure) { m_magboltz.SetPressure(pressure/1013.25*760);}
    Garfield::MediumMagboltz* getMediumMagbolz() { return &m_magboltz;}
  private:
    Garfield::MediumMagboltz m_magboltz;
  };
}