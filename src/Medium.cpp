#include "RPCSim/Medium.hpp"
#include "TGeoMedium.h"
#include "TGeoManager.h"

TGeoElementTable* RPCSim::Medium::m_elementTable{nullptr};

RPCSim::Medium::Medium()
{
  getElementTable();
}

void RPCSim::Medium::getElementTable()
{
  if(gGeoManager==nullptr) gGeoManager=new TGeoManager();
  m_elementTable=gGeoManager->GetElementTable();
}

RPCSim::Glass::Glass()
{
  // https://www.fe.infn.it/u/paterno/Geant4_tutorial/slides_further/Geometry/G4_Nist_Materials.pdf
  static TGeoMixture* glass= new TGeoMixture("Glass",4,2.4);
  glass->AddElement(m_elementTable->GetElement(8),0.4598);
  glass->AddElement(m_elementTable->GetElement(11),0.0964411);
  glass->AddElement(m_elementTable->GetElement(14),0.336553);
  glass->AddElement(m_elementTable->GetElement(20),0.107205);
  m_medium = new TGeoMedium("ElectrodeMedium",1,glass);
}

RPCSim::Graphite::Graphite()
{
  static TGeoMixture* graphite= new TGeoMixture("Graphite",1,1.7);
  graphite->AddElement(m_elementTable->GetElement(6),1.0);
  m_medium = new TGeoMedium("Graphite",1,graphite);
}

RPCSim::Mylar::Mylar()
{
  static TGeoMixture* mylar= new TGeoMixture("Mylar",3,1.4);
  mylar->AddElement(m_elementTable->GetElement(1),0.041959);
  mylar->AddElement(m_elementTable->GetElement(6),0.625016);
  mylar->AddElement(m_elementTable->GetElement(8),0.333025);
  m_medium = new TGeoMedium("Mylar",1,mylar);
}