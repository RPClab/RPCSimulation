#include <TApplication.h>
#include <TGeoManager.h>
#include <TROOT.h>


#include "Garfield/GeometryRoot.hh"
#include "Garfield/AvalancheGrid.hh"
#include "Garfield/AvalancheMC.hh"
#include "Garfield/ComponentAnalyticField.hh"
#include "Garfield/Plotting.hh"
#include "Garfield/SolidBox.hh"
#include "Garfield/TrackHeed.hh"
#include "Garfield/ViewSignal.hh"
#include "Garfield/ViewField.hh"
#include "Garfield/ViewGeometry.hh"


#include "RPCSim/DefaultFolders.hpp"
#include "RPCSim/Medium.hpp"
#include "RPCSim/GasMixture.hpp"

using namespace Garfield;

int main(int argc, char *argv[]) {
  // Setup all this mess
  RPCSim::setEnv();
  TApplication app("app", &argc, argv);
  plottingEngine.SetDefaultStyle();

  // Setup TGeoManager
  TGeoManager *geom = new TGeoManager("RPCSim", "RPC Simulation");

  // Setup the gas, but one can also use a gasfile.
  RPCSim::GasMixture gasMixture(RPCSim::data_folder+"/examples/c2h2f4_94-7_iso_5_sf6_0-3_bis.gas");
  //--- define some media
  TGeoMedium* vacuum = new TGeoMedium("Vacuum",1,new TGeoMaterial("Vacuum", 0,0,0));
  TGeoVolume* universe = geom->MakeBox("Universe", vacuum, 270., 270., 120.);
  geom->SetTopVolume(universe);

  //TGeoElementTable* table = gGeoManager->GetElementTable();
  // https://www.fe.infn.it/u/paterno/Geant4_tutorial/slides_further/Geometry/G4_Nist_Materials.pdf
  //TGeoMixture* glass= new TGeoMixture("Glass",4,2.4);
  //glass->AddElement(table->GetElement(8),0.4598);
  //glass->AddElement(table->GetElement(11),0.0964411);
  //glass->AddElement(table->GetElement(14),0.336553);
  //glass->AddElement(table->GetElement(20),0.107205);
  //TGeoMedium* electrode_medium = new TGeoMedium("ElectrodeMedium",1,glass);
  RPCSim::Glass glass;

  TGeoVolume* anode = gGeoManager->MakeBox("Anode",glass(),5,0.055,5); //1m*1m*1.1mm
  anode->SetLineColor(kBlue);
  anode->SetTransparency(75);
  TGeoVolume* cathode = gGeoManager->MakeBox("Cathode",glass(),5,0.055,5); //1m*1m*1.1mm
  cathode->SetLineColor(kBlue);
  cathode->SetTransparency(75);

  universe->AddNode(anode,1,new TGeoTranslation(0,0,0));
  universe->AddNode(cathode,1,new TGeoTranslation(0,0.21,0));

  TGeoVolume* gas = gGeoManager->MakeBox("Gas",gasMixture(),5,0.05,5); //1m*1m*1mm
  gas->SetLineColor(kRed);
  gas->SetTransparency(75);
  universe->AddNode(gas,1,new TGeoTranslation(0,0.105,0));



  geom->CloseGeometry();
  //universe->Draw("ogl");




  Garfield::GeometryRoot* geo = new GeometryRoot();
  // Pass the pointer to the TGeoManager.
  geo->SetGeometry(geom);
  geo->SetMedium("Gas", gasMixture.getMagboltzMedium());

  Garfield::ComponentAnalyticField cmp;
  cmp.SetMedium(gasMixture.getMagboltzMedium());
  cmp.AddPlaneY(0,0,"Anode");
  cmp.AddPlaneY(0.32,-6400,"Cathode");
  constexpr double pitch = 4;
  constexpr double halfpitch = 0.5 * pitch;
  cmp.AddStripOnPlaneY('z', 0.32, -halfpitch, halfpitch, "strip");


  Garfield::Sensor sensor;
  // Calculate the electric field using the Component object cmp.
  sensor.AddComponent(&cmp);
  // Request signal calculation for the electrode named "s",
  // using the weighting field provided by the Component object cmp.
  sensor.AddElectrode(&cmp, "strip");
  const unsigned int nTimeBins = 10000;
  const double tmin = 0.;
  const double tmax = 20.;
  const double tstep = (tmax - tmin) / nTimeBins;
  sensor.SetTimeWindow(tmin, tstep, nTimeBins);







Garfield::AvalancheMC drift;
drift.SetSensor(&sensor);
// Set the step size [cm].
drift.SetDistanceSteps(1.e-4);


Garfield::TrackHeed track;
track.SetSensor(&sensor);
// Set the particle type and momentum [eV/c].
track.SetParticle("muon");
track.SetMomentum(1800.0e6);
double x0 = 0., y0 = 0., z0 = 0., t0 = 0.;
double dx = 0., dy = 1., dz = 0.;
track.NewTrack(x0, y0, z0, t0, dx, dy, dz);
Garfield::ViewDrift driftView;
//track.EnablePlotting(&driftView);
drift.EnablePlotting(&driftView);
std::cout<<"Number of clusters :"<<track.GetClusters().size()<<std::endl;
// Retrieve the clusters along the track.
for(const auto& cluster : track.GetClusters())
{
  std::size_t i=0;
  // Loop over the electrons in the cluster.
  for(const auto& electron : cluster.electrons)
  {
    drift.DriftElectron(electron.x, electron.y, electron.z, electron.t);
  }
  driftView.Plot3d();
}









  //app.Run(kTRUE);
}
