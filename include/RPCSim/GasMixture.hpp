#ifndef RPCSIM_GASMIXTURE_H_
#define RPCSIM_GASMIXTURE_H_

#include <string>
#include "RPCSim/Medium.hpp"
#include "Garfield/MediumMagboltz.hh"

namespace RPCSim
{

class GasMixture : public Medium
{
public:
  GasMixture(const std::string& gas1, const double& f1,const std::string& gas2=std::string(), const double& f2=0.,const std::string& gas3=std::string(), const double& f3=0.,const std::string& gas4=std::string(), const double& f4=0.,const std::string& gas5=std::string(), const double& f5=0.,const std::string& gas6=std::string(), const double& f6=0.);
  GasMixture(const std::string& path);
  Garfield::MediumMagboltz* getMagboltzMedium() { return &m_magmedium;}
  virtual ~GasMixture() = default;
private:
  Garfield::MediumMagboltz m_magmedium;
  bool m_isFile{false};
};

}

#endif