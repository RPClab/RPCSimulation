#include "RPCSim/GasMixture.hpp"
#include <TGeoMedium.h>

RPCSim::GasMixture::GasMixture(const std::string& gas1, const double& f1,const std::string& gas2, const double& f2,const std::string& gas3, const double& f3,const std::string& gas4, const double& f4,const std::string& gas5, const double& f5,const std::string& gas6, const double& f6) : m_isFile(false)
{
  m_magmedium.SetComposition(gas1, f1, gas2, f2, gas3, f3, gas4, f4, gas5, f5, gas6, f6);
  m_medium = new TGeoMedium("Gas",1,new TGeoMaterial("Gas",m_magmedium.GetAtomicWeight(),m_magmedium.GetAtomicNumber(),m_magmedium.GetMassDensity(), TGeoMaterial::EGeoMaterialState::kMatStateGas, m_magmedium.GetTemperature(), m_magmedium.GetPressure()/760.));
}

RPCSim::GasMixture::GasMixture(const std::string& path) : m_isFile(true)
{
  m_magmedium.LoadGasFile(path);
  m_magmedium.Initialise();
  m_medium = new TGeoMedium("Gas",1,new TGeoMaterial("Gas",m_magmedium.GetAtomicWeight(),m_magmedium.GetAtomicNumber(),m_magmedium.GetMassDensity(), TGeoMaterial::EGeoMaterialState::kMatStateGas, m_magmedium.GetTemperature(), m_magmedium.GetPressure()/760.));
}

void RPCSim::GasMixture::generate(const double& E, const double& B, const double& angle)
{
  m_magmedium.SetFieldGrid({E},{B},{angle});
  m_magmedium.EnableThermalMotion(true);
  m_magmedium. GenerateGasTable(10,true);
}