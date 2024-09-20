#include "RPCSim/new/GasMixture.hpp"

RPCSim::GasMixture::GasMixture(const std::string& filename)
{
  setFile(filename);
}

RPCSim::GasMixture::GasMixture(const std::vector<std::string>& gases, const std::vector<double>& percent)
{
  setMixture(gases,percent);
}

void RPCSim::GasMixture::setFile(const std::string& filename)
{
  m_magboltz.LoadGasFile(filename);
}

void RPCSim::GasMixture::setMixture(const std::vector<std::string>& names, const std::vector<double>& percent)
{
  std::vector<std::string> names_full{names};
  names_full.resize(6);
  std::vector<double> percent_full{percent};
  percent_full.resize(6);
  m_magboltz.SetComposition(names_full[0],percent_full[0],names_full[1],percent_full[1],names_full[2],percent_full[2],names_full[3],percent_full[3],names_full[4],percent_full[4],names_full[5],percent_full[5]);
}