#include "RPCSim/RPCGeometry.hpp"
#include "RPCSim/Medium.hpp"
#include "RPCSim/GasMixture.hpp"
#include <TGeoManager.h>
#include <Garfield/GeometryRoot.hh>
#include <iostream>


RPCSim::RPCGeometry::RPCGeometry()
{
  if(gGeoManager==nullptr) static TGeoManager *geom = new TGeoManager("RPCSim", "RPC Simulation"); //Should be in other place !!
  else static TGeoManager *geom= gGeoManager;
}

void RPCSim::RPCGeometry::setElectrodeMaterial(const RPCSim::Medium& medium)
{
  m_electrode_medium=medium;
}

void RPCSim::RPCGeometry::fillGasGap(RPCSim::GasMixture* gas_mixture)
{
  m_gas_mixture=gas_mixture;
}

void RPCSim::RPCGeometry::draw(const std::string& options)
{
  if(gGeoManager->GetTopVolume()==nullptr) std::cout<<"Top Volume not defined !!! What to print dude !!!"<<std::endl;
  else gGeoManager->GetTopVolume()->Draw(options.c_str());
}

void RPCSim::RPCGeometry::build()
{
  TGeoMedium* vacuum = new TGeoMedium("Vacuum",1,new TGeoMaterial("Vacuum", 0,0,0));
  TGeoVolume* universe = gGeoManager->MakeBox("Universe", vacuum, 270., 270., 120.);
  gGeoManager->SetTopVolume(universe);
  RPCSim::Glass glass;
  TGeoVolume* anode = gGeoManager->MakeBox("Anode",m_electrode_medium(),m_length/2.,m_electrodeThickness/2.,m_width/2.0); //1m*1m*1.1mm
  anode->SetLineColor(kBlue);
  anode->SetTransparency(75);
  TGeoVolume* cathode = gGeoManager->MakeBox("Cathode",m_electrode_medium(),m_length/2,m_electrodeThickness/2.0,m_width/2.0); //1m*1m*1.1mm
  cathode->SetLineColor(kBlue);
  cathode->SetTransparency(75);
  universe->AddNode(anode,1,new TGeoTranslation(0,0,0));
  universe->AddNode(cathode,1,new TGeoTranslation(0,m_gasGapThickness+2*(m_electrodeThickness/2),0));
  TGeoVolume* gas = gGeoManager->MakeBox("Gas",m_gas_mixture->operator()(),m_length/2,m_gasGapThickness/2.,m_width/2.0); //1m*1m*1mm
  gas->SetLineColor(kRed);
  gas->SetTransparency(30);
  universe->AddNode(gas,1,new TGeoTranslation(0,m_gasGapThickness/2+m_electrodeThickness/2.0,0));
  gGeoManager->CloseGeometry();
}

Garfield::Geometry* RPCSim::RPCGeometry::getGeometry()
{
  build(); //GOOD OR NOT
  static Garfield::GeometryRoot* geo = new Garfield::GeometryRoot();
  // Pass the pointer to the TGeoManager.
  geo->SetGeometry(gGeoManager);
  geo->SetMedium("Gas", m_gas_mixture->getMagboltzMedium());
  return geo;
}


