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

RPCSim::FR4::FR4()
{
  // Create Epoxy material
  static TGeoMixture* epoxy= new TGeoMixture("Epoxy",3,1.3);
  epoxy->AddElement(m_elementTable->GetElement(1),44);
  epoxy->AddElement(m_elementTable->GetElement(6),15);
  epoxy->AddElement(m_elementTable->GetElement(8),7);
  static TGeoMedium* epoxy_medium = new TGeoMedium("Epoxy",1,epoxy);
  // SiO2
  static TGeoMixture* sio2= new TGeoMixture("SiO2",2,2.20);
  sio2->AddElement(m_elementTable->GetElement(14),1);
  sio2->AddElement(m_elementTable->GetElement(8),2);
  static TGeoMedium* sio2_medium = new TGeoMedium("SiO2",1,sio2);

  static TGeoMixture* fr4= new TGeoMixture("FR4",2,1.85);
  fr4->AddElement(epoxy,0.472);
  fr4->AddElement(sio2,0.528);
  m_medium = new TGeoMedium("FR4",1,fr4);
}

RPCSim::Copper::Copper()
{
  static TGeoMaterial *copper = new TGeoMaterial("Copper",63.546,29,8.935);
  m_medium = new TGeoMedium("Copper",1,copper);
}