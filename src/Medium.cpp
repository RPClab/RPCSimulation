#include "RPCSim/Medium.hpp"
#include "TGeoMedium.h"
#include "TGeoElement.h"

TGeoElementTable RPCSim::Medium::m_elementTable{TGeoElementTable(200)};

RPCSim::Glass::Glass()
{
  // https://www.fe.infn.it/u/paterno/Geant4_tutorial/slides_further/Geometry/G4_Nist_Materials.pdf
  static TGeoMixture* glass= new TGeoMixture("Glass",4,2.4);
  glass->AddElement(m_elementTable.GetElement(8),0.4598);
  glass->AddElement(m_elementTable.GetElement(11),0.0964411);
  glass->AddElement(m_elementTable.GetElement(14),0.336553);
  glass->AddElement(m_elementTable.GetElement(20),0.107205);
  m_medium = new TGeoMedium("ElectrodeMedium",1,glass);
}