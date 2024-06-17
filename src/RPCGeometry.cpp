#include "RPCSim/RPCGeometry.hpp"
#include "RPCSim/Medium.hpp"
#include "RPCSim/GasMixture.hpp"
#include <Rtypes.h>
#include <TGeoManager.h>
#include "RPCSim/PCBGeometry.hpp"
#include <Garfield/GeometryRoot.hh>
#include <TGeoBBox.h>
#include <iostream>

void RPCSim::RPCGeometry::putOnTop(PCBGeometry& top)
{
  top.setType(PCBGeometry::Type::Top);
  TGeoTranslation translate(0,m_dimensions.heigth(),0);
  top.translate(translate);
  top.setLength(m_dimensions[0]);
  top.setWidth(m_dimensions[1]);
  top.build();
}

void RPCSim::RPCGeometry::putOnBottom(PCBGeometry& bottom) //FIXME
{
  bottom.setType(PCBGeometry::Type::Bottom);
  TGeoTranslation translate(0,0,0);
  bottom.translate(translate);
  bottom.setLength(m_dimensions[0]);
  bottom.setWidth(m_dimensions[1]);
  bottom.build();
}

void RPCSim::RPCGeometry::setHeight()
{
  m_dimensions[2]=  m_gasGapThickness+2*m_electrodeThickness+2*m_graphiteThickness+2*m_mylarThickness;
}

RPCSim::RPCGeometry::RPCGeometry()
{
  if(gGeoManager==nullptr) static TGeoManager *geom = new TGeoManager("RPCSim", "RPC Simulation"); //Should be in other place !!
  else static TGeoManager *geom= gGeoManager;
  setLength();
  setWidth();
  setHeight();
}

void RPCSim::RPCGeometry::setElectrodeMaterial(const RPCSim::Medium& medium)
{
  m_electrode_medium=medium;
}

void RPCSim::RPCGeometry::fillGasGap(RPCSim::GasMixture* gas_mixture)
{
  m_gas_mixture=gas_mixture;
}

void RPCSim::RPCGeometry::closeGeometry()
{
  gGeoManager->CloseGeometry();
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
  gGeoManager->SetCurrentPoint(0.,0.,0.);
  RPCSim::Glass glass;
  RPCSim::Graphite graphite;
  RPCSim::Mylar mylar;
  // TODO Check anode and cathode name I'm always confused about such Chemist words !!!
  double position = m_mylarThickness/2.; // Position of the bottom;
  // Mylar
  TGeoVolume* mylar_layer_1 = gGeoManager->MakeBox("MylarLayerAnode",mylar(),m_dimensions.length()/2.,m_mylarThickness/2.,m_dimensions.width()/2.0); //1m*1m*200um
  mylar_layer_1->SetLineColor(kCyan-9);
  mylar_layer_1->SetTransparency(30);
  universe->AddNode(mylar_layer_1,1,new TGeoTranslation(0,position,0));
  m_HVAnodePosition=m_mylarThickness+m_graphiteThickness;
  m_startGasGap=m_mylarThickness+m_graphiteThickness+m_electrodeThickness;
  m_endGasGap=m_mylarThickness+m_graphiteThickness+m_electrodeThickness+m_gasGapThickness;
  position+=m_graphiteThickness/2+m_mylarThickness/2;
  // Graphite layer
  TGeoVolume* graphite_layer_1 = gGeoManager->MakeBox("GraphiteLayerAnode",graphite(),m_dimensions.length()/2.,m_graphiteThickness/2.,m_dimensions.width()/2.0); //1m*1m*200um
  graphite_layer_1->SetLineColor(kGray+3);
  graphite_layer_1->SetTransparency(0);
  universe->AddNode(graphite_layer_1,1,new TGeoTranslation(0,position,0));
  position+=m_graphiteThickness/2+m_electrodeThickness/2.;
  // Anode
  TGeoVolume* anode = gGeoManager->MakeBox("Anode",m_electrode_medium(),m_dimensions.length()/2.,m_electrodeThickness/2.,m_dimensions.width()/2.0);
  anode->SetLineColor(kAzure+1);
  anode->SetTransparency(10);
  universe->AddNode(anode,1,new TGeoTranslation(0,position,0));
  position+=m_electrodeThickness/2.+m_gasGapThickness/2;
  // Gas
  TGeoVolume* gas = gGeoManager->MakeBox("Gas",m_gas_mixture->operator()(),m_dimensions.length()/2.,m_gasGapThickness/2.,m_dimensions.width()/2.0);
  gas->SetLineColor(kWhite);
  gas->SetTransparency(80);
  universe->AddNode(gas,1,new TGeoTranslation(0,position,0));
  // Cathode
  position+=m_electrodeThickness/2.+m_gasGapThickness/2;
  TGeoVolume* cathode = gGeoManager->MakeBox("Cathode",m_electrode_medium(),m_dimensions.length()/2.,m_electrodeThickness/2.0,m_dimensions.width()/2.0);
  cathode->SetLineColor(kAzure+1);
  cathode->SetTransparency(10);
  universe->AddNode(cathode,1,new TGeoTranslation(0,position,0));
  position+=m_graphiteThickness/2.+m_electrodeThickness/2.;
  // Graphite layer
  TGeoVolume* graphite_layer_2 = gGeoManager->MakeBox("GraphiteLayerCathode",graphite(),m_dimensions.length()/2.,m_graphiteThickness/2.,m_dimensions.width()/2.0);
  graphite_layer_2->SetLineColor(kGray+3);
  graphite_layer_2->SetTransparency(0);
  universe->AddNode(graphite_layer_2,1,new TGeoTranslation(0,position,0));
  position+=m_graphiteThickness/2.+m_mylarThickness/2.;
  // Mylar
  TGeoVolume* mylar_layer_2 = gGeoManager->MakeBox("MylarLayerCathode",mylar(),m_dimensions.length()/2.,m_mylarThickness/2.,m_dimensions.width()/2.0);
  mylar_layer_2->SetLineColor(kCyan-9);
  mylar_layer_2->SetTransparency(30);
  universe->AddNode(mylar_layer_2,1,new TGeoTranslation(0,position,0));
  m_HVCathodePosition=position+m_mylarThickness/2.-m_graphiteThickness-m_mylarThickness;
}

  double RPCSim::RPCGeometry::HVAnodePosition()
  {
    return m_HVAnodePosition;
  }

  double RPCSim::RPCGeometry::HVCathodePosition()
  {
    return m_HVCathodePosition;
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


  double RPCSim::RPCGeometry::startGasGap()
  {
    return m_startGasGap;
  }

  double RPCSim::RPCGeometry::endGasGap()
  {
    return m_endGasGap;
  }