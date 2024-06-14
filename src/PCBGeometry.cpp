#include "RPCSim/PCBGeometry.hpp"
#include <TGeoManager.h>
#include "RPCSim/Medium.hpp"

RPCSim::PCBGeometry::PCBGeometry()
{
  if(gGeoManager==nullptr) static TGeoManager *geom = new TGeoManager("RPCSim", "RPC Simulation"); //Should be in other place !!
  else static TGeoManager *geom= gGeoManager;
  setLength();
  setWidth();
  setHeight();
}

void RPCSim::PCBGeometry::setHeight()
{
  m_dimensions[2]=m_strip_thickness+m_FR4_thickness_strip+m_honeycomb_thickness+m_FR4_thickness_ground+m_ground_thickness;
}

void RPCSim::PCBGeometry::build()
{
  RPCSim::FR4 fr4;
  RPCSim::Mylar mylar;
  RPCSim::Copper copper;
  double position = 0; // Position of the bottom;
  // Strip Layer
  TGeoVolume* strip_layer = gGeoManager->MakeBox("Strip_layer",copper(),m_dimensions.length()/2.,m_strip_thickness/2.,m_dimensions.width()/2.0);
  strip_layer->SetLineColor(kOrange);
  strip_layer->SetTransparency(0);
  gGeoManager->GetTopVolume()->AddNode(strip_layer,1,new TGeoTranslation(m_translation.GetTranslation()[0],m_translation.GetTranslation()[1]+position,m_translation.GetTranslation()[2]));
  position+=m_strip_thickness/2+m_FR4_thickness_strip/2;
  // FR4 layer
  TGeoVolume* FR4_layer_1 = gGeoManager->MakeBox("FR4_layer_1",mylar(),m_dimensions.length()/2.,m_FR4_thickness_strip/2.,m_dimensions.width()/2.0);
  FR4_layer_1->SetLineColor(kGreen);
  FR4_layer_1->SetTransparency(0);
  gGeoManager->GetTopVolume()->AddNode(FR4_layer_1,1,new TGeoTranslation(m_translation.GetTranslation()[0],m_translation.GetTranslation()[1]+position,m_translation.GetTranslation()[2]));
  position+=m_FR4_thickness_strip/2+m_honeycomb_thickness/2.;
  // Honeycomb
  static TGeoMaterial *vacuum = new TGeoMaterial("vacuum",0,0,0);
  static TGeoMedium *Air = new TGeoMedium("Air",0,vacuum);
  TGeoVolume* honeycomb = gGeoManager->MakeBox("Honeycomb",Air,m_dimensions.length()/2.,m_honeycomb_thickness/2.,m_dimensions.width()/2.0);
  honeycomb->SetLineColor(kBlue);
  honeycomb->SetTransparency(80);
  gGeoManager->GetTopVolume()->AddNode(honeycomb,1,new TGeoTranslation(m_translation.GetTranslation()[0],m_translation.GetTranslation()[1]+position,m_translation.GetTranslation()[2]));
  position+=m_honeycomb_thickness/2.+m_FR4_thickness_strip/2;
  // FR4 layer
  TGeoVolume* FR4_layer_2 = gGeoManager->MakeBox("FR4_layer_2",mylar(),m_dimensions.length()/2.,m_FR4_thickness_strip/2.0,m_dimensions.width()/2.0);
  FR4_layer_2->SetLineColor(kGreen);
  FR4_layer_2->SetTransparency(0);  
  gGeoManager->GetTopVolume()->AddNode(FR4_layer_2,1,new TGeoTranslation(m_translation.GetTranslation()[0],m_translation.GetTranslation()[1]+position,m_translation.GetTranslation()[2]));
  position+=m_FR4_thickness_strip/2.+m_strip_thickness/2.;
  // Ground layer
  TGeoVolume* ground = gGeoManager->MakeBox("Ground",copper(),m_dimensions.length()/2.,m_ground_thickness/2.,m_dimensions.width()/2.0);
  ground->SetLineColor(kBlack);
  ground->SetTransparency(0);
  gGeoManager->GetTopVolume()->AddNode(ground,1,new TGeoTranslation(m_translation.GetTranslation()[0],m_translation.GetTranslation()[1]+position,m_translation.GetTranslation()[2]));
  setHeight();
}