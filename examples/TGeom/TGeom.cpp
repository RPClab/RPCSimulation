#include <TApplication.h>
#include <TGeoManager.h>
#include <TROOT.h>
#include <TSystem.h>
#include <string>


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
#include "Garfield/ViewCell.hh"


#include "RPCSim/DefaultFolders.hpp"
#include "RPCSim/Exceptions.hpp"
#include "RPCSim/Setup.hpp"
#include "RPCSim/RPCGeometry.hpp"
#include "RPCSim/PCBGeometry.hpp"


#define LOG(x) std::cout << x << std::endl


using namespace Garfield;

int main(int argc, char *argv[]) try
{
  // Setup all this mess
  RPCSim::setEnv();
  TApplication app("app", &argc, argv);
  //plottingEngine.SetDefaultStyle();

  // Top strip pannel
  RPCSim::PCBGeometry top_strip_pannel;

  // Bottom strip pannel
  RPCSim::PCBGeometry bottom_strip_pannel;

  RPCSim::RPCGeometry my_rpc;
  // Setup the gas, but one can also use a gasfile.
  RPCSim::GasMixture gasMixture(RPCSim::data_folder+"/examples/c2h2f4_94-7_iso_5_sf6_0-3_bis.gas");
  //gasMixture.generate(7000.0,0.,0.);
  my_rpc.fillGasGap(&gasMixture);
  my_rpc.build();
  my_rpc.putOnTop(top_strip_pannel);
  my_rpc.putOnBottom(bottom_strip_pannel);
  my_rpc.closeGeometry();
  my_rpc.draw();
  std::cout<<"Creating RPC with length : "<<my_rpc.getDimensions().length()<<" width : "<<my_rpc.getDimensions().width()<<" thickness : "<<my_rpc.getDimensions().heigth()<<std::endl;
  
  // Create electric field and strips
  Garfield::ComponentAnalyticField cmp;
  cmp.SetMedium(gasMixture.getMagboltzMedium());
  cmp.AddPlaneY(my_rpc.HVAnodePosition(),0,"Anode");
  cmp.AddPlaneY(my_rpc.HVCathodePosition(),-6600,"Cathode");
  constexpr double pitch = 4;
  constexpr double extar=0.1;
  const std::size_t nbr_strips{12};
  for(std::size_t i=0;i!=nbr_strips;++i)
  {
    cmp.AddStripOnPlaneY('z', 0.0, i*pitch+(i*extar), (i+1)*pitch+(i*extar), "strip_"+std::to_string(i));
  }
  ViewField fieldView;
  fieldView.SetComponent(&cmp);
  // Plot the potential along the hole axis.
  fieldView.PlotProfile(-50., 50.,-0.05, 0.38, -50.0, 50.0);



  const bool debug = true;
  constexpr bool plotSignal = true;

  Garfield::Sensor sensor;
  // Calculate the electric field using the Component object cmp.
  sensor.AddComponent(&cmp);
  // Request signal calculation for the electrode named "s",
  // using the weighting field provided by the Component object cmp.
  for(std::size_t i=0;i!=12;++i)
  {
    sensor.AddElectrode(&cmp, "strip_"+std::to_string(i));
  }
  // Set the time bins.
  const unsigned int nTimeBins = 1000;
  const double tmin = 0.;
  const double tmax = 100;
  const double tstep = (tmax - tmin) / nTimeBins;
  sensor.SetTimeWindow(tmin, tstep, nTimeBins);

  // Create the AvalancheMicroscopic.
  AvalancheMicroscopic aval;
  aval.SetSensor(&sensor);
  aval.EnableSignalCalculation();
  aval.UseWeightingPotential();

  // Set time window where the calculations will be done microscopically.
  const double tMaxWindow = 2000;
  aval.SetTimeWindow(0., tMaxWindow);

  // Preparing the plotting of the induced charge and signal of the electrode
  // readout.
  ViewSignal *signalView = nullptr;
  TCanvas *cSignal = nullptr;
  if (plotSignal) {
    cSignal = new TCanvas("cSignal", "", 600, 600);
    signalView = new ViewSignal();
    signalView->SetCanvas(cSignal);
    signalView->SetSensor(&sensor);
  }

  ViewSignal *chargeView = nullptr;
  TCanvas *cCharge = nullptr;

  if (plotSignal) {
    cCharge = new TCanvas("cCharge", "Charge", 600, 600);
    chargeView = new ViewSignal();
    chargeView->SetCanvas(cCharge);
    chargeView->SetSensor(&sensor);
  }


  Garfield::ViewDrift view_drift;
  // Set up Heed.
  TrackHeed track;
  //track.EnableDeltaElectronTransport();
  track.SetSensor(&sensor);
  // Set the particle type and momentum [eV/c].
  track.SetParticle("muon");
  track.SetMomentum(100.e6);

  // Setting the timer for the running time of the algorithm.
  std::clock_t start = std::clock();

  std::cout<<"Start gas gap ="<<my_rpc.startGasGap()<<" end="<<my_rpc.endGasGap()<<std::endl;
  // Simulate a charged-particle track.
  track.NewTrack(0, my_rpc.endGasGap(), 0, 0, 0, -1, 0);
  // Retrieve the clusters along the track.
  for (const auto &cluster : track.GetClusters()) {
    // Loop over the electrons in the cluster.
    for (const auto &electron : cluster.electrons) {
      // Simulate the electron track
      aval.AvalancheElectron(electron.x, electron.y, electron.z, electron.t,electron.e,electron.dx,electron.dy, electron.dz);
    }
  }

  double duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;

  LOG("Script: Electrons have drifted. It took " << duration << "s to run.");


  if (plotSignal) {
    // Plot signals
    signalView->PlotSignal("strip_0");
    cSignal->Update();
    //gSystem->ProcessEvents();

    // Plot induced charge
    sensor.IntegrateSignal("strip_0");
    chargeView->PlotSignal("strip_0");
    cCharge->Update();
    //gSystem->ProcessEvents();
  }
  LOG("Script: Total induced charge = " << sensor.GetTotalInducedCharge("strip_0")<< " [fC].");

  app.Run(kTRUE);















}
catch(const RPCSim::exception& exception)
{
  std::cout<<"RPCSim::exception : "<<exception.what()<<std::endl;
}
catch(const std::exception& exception)
{
  std::cout<<"std::exception : "<<exception.what()<<std::endl;
}
catch(...)
{
  return -1;
}
