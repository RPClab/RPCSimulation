#include <TApplication.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TH1.h>
#include <TROOT.h>
#include <TSystem.h>

#include <ctime>
#include <iostream>

#include "Garfield/AvalancheGrid.hh"
#include "Garfield/AvalancheMicroscopic.hh"
#include "Garfield/ComponentParallelPlate.hh"
#include "Garfield/ComponentAnalyticField.hh"
#include "Garfield/FundamentalConstants.hh"
#include "Garfield/GeometrySimple.hh"
#include "Garfield/MediumMagboltz.hh"
#include "Garfield/Plotting.hh"
#include "Garfield/Sensor.hh"
#include "Garfield/SolidBox.hh"
#include "Garfield/TrackHeed.hh"
#include "Garfield/ViewSignal.hh"
#include "Garfield/ViewField.hh"
#include "Garfield/ViewCell.hh"
#include "Garfield/MediumPlastic.hh"

#include "RPCSim/DefaultFolders.hpp"

#define LOG(x) std::cout << x << std::endl

using namespace Garfield;

int main(int argc, char *argv[]) {
  RPCSim::setEnv();
  TApplication app("app", &argc, argv);
  plottingEngine.SetDefaultStyle();

  const bool debug = true;
  constexpr bool plotSignal = true;

  // The geometry of the RPC.
  const int N = 3;  // Total amount of layers inside the geometry

  // Relative permitivity of the layers
  const double epGlass = 4.2;     // [1]
  const double epGas = 1.;        // [1]

  std::vector<double> eps = {epGlass, epGas, epGlass};
  // Thickness of the layers
  const double dGlass = 0.11;      // [cm]
  const double dGas = 0.1;        // [cm]
  std::vector<double> thickness = {dGlass, dGas, dGlass};
  double totalThickness = dGlass + dGas + dGlass;

  // Applied potential
  const double voltage = -6000;  // [V]
  ComponentParallelPlate *RPC = new ComponentParallelPlate();
  RPC->Setup(N, eps, thickness, voltage);

  // Adding a readout structure.
  const std::string label = "strip4";


  // Setup the gas, but one can also use a gasfile.
  MediumMagboltz gas;
  gas.LoadGasFile(RPCSim::data_folder+"/examples/c2h2f4_94-7_iso_5_sf6_0-3_bis.gas");
  gas.Initialise(true);

  //Setup the Geometry
  SolidBox box1(0.,dGlass/2,0., 20., dGlass / 2, 20.);
  SolidBox box(0., dGlass + dGas / 2, 0., 20., dGas / 2, 20.);
  SolidBox box2(0., dGlass + dGas + dGlass / 2, 0., 20., dGlass / 2, 20.);
  GeometrySimple geo;
  MediumPlastic glass;
  glass.SetDielectricConstant(4.2);
  geo.AddSolid(&box1, &glass);
  geo.AddSolid(&box, &gas);
  geo.AddSolid(&box2, &glass);
  RPC->SetGeometry(&geo);

  ComponentAnalyticField wField;
  wField.SetGeometry(&geo);
  wField.AddPlaneY(0, 1, "wleft");
  wField.AddPlaneY(dGlass + dGas + dGlass, 0, "wright");
  for (int i=0;i<9;i++)
  {
    std::string strip_label = "strip";
    std::string strip_labeli = strip_label+std::to_string(i);
    wField.AddStripOnPlaneY('z' , 0 , 4*(i-5) + 2.1 , 4*(i-4)-0.2 + 2.1 , strip_labeli);
    wField.AddReadout(strip_labeli);
  }

  // Create the sensor.
  Sensor sensor;
  sensor.AddComponent(RPC);
  for (int i=0;i<9;i++)
  {
    std::string strip_label = "strip";
    std::string strip_labeli = strip_label+std::to_string(i);
    sensor.AddElectrode(&wField, strip_labeli);
  }

  /*TCanvas canvas("c", "", 600, 600);
  ViewField fieldView;
  fieldView.SetCanvas(&canvas);
  fieldView.SetSensor(&sensor);
  fieldView.SetArea(-30, -0.05, -30, 30,totalThickness+0.05, 30);
  //fieldView.SetPlane(0, 0, 1, 0, 0, 0);
  fieldView.PlotContourWeightingField("ReadoutPlane", "v");
  gSystem->ProcessEvents();*/


  // Set the time bins.
  const unsigned int nTimeBins = 30000;
  const double tmin = 0.;
  const double tmax = 4;
  const double tstep = 0.001;//(tmax - tmin) / nTimeBins;
  sensor.SetTimeWindow(tmin, tstep, nTimeBins);




  /// Get the drift field and potential at (x, y, z).
  double x = 0.0, y = dGlass , z = 0;
  double ex = 0.0, ey = 0.0, ez = 0.0, v = 0.0;
  Medium* medium = nullptr;
  int status = 0;
  sensor.ElectricField(x, y, z, ex, ey, ez, v, medium, status);

  std::cout << "Electric field at (" << x << ", " << y << ", " << z << "): (" << ex << ", " << ey << ", " << ez << ") V/cm" << std::endl;
  std::cout << "Electric potential at (" << x << ", " << y << ", " << z << "): " << v << "V" << std::endl;
  //std::cout<< "Weighting Potential at (" << x << ", " << y << ", " << z << "): " <<label<<": "<< sensor.WeightingPotential(x,y,z,label) << "V" << std::endl;
  for (int i=0;i<9;i++)
  {
    std::string strip_label = "strip";
    std::string strip_labeli = strip_label+std::to_string(i);
    std::cout<< "Weighting Potential at (" << x << ", " << y << ", " << z << "): " <<strip_labeli<<": "<< sensor.WeightingPotential(x,y,z,strip_labeli) << "V" << std::endl;
  }

  TCanvas canvas1("c1", "", 600, 600);
  for (int i = 0; i < 1; i++)
  {
    std::string strip_label = "strip";
    std::string strip_labeli = strip_label + std::to_string(i);
    std::string canvas_i = "canvas_" + strip_labeli;
    //TCanvas canvasi(canvas_i.c_str(), "", 600, 600);
    ViewField fieldView;
    fieldView.SetCanvas(&canvas1);
    fieldView.SetSensor(&sensor);
    fieldView.SetArea(-22, -0.1, 22, 0.4);
    fieldView.PlotContourWeightingField(strip_labeli, "v");
    //gSystem->ProcessEvents();
  }

  // Create the AvalancheMicroscopic.
  AvalancheMicroscopic aval;
  aval.SetSensor(&sensor);
  aval.EnableSignalCalculation();
  aval.UseWeightingPotential();
  
  // Set time window where the calculations will be done microscopically.
  const double tMaxWindow = 0.8;
  aval.SetTimeWindow(0., tMaxWindow);

  // Create the AvalancheGrid for grid-based avalanche calculations that are
  // suited for constant drift fields. This class will take over the
  // calculations of the microscopic class after the set time-window.
  AvalancheGrid avalgrid;
  avalgrid.SetSensor(&sensor);
  avalgrid.SetMaxAvalancheSize(1.6e8);
  avalgrid.SetElectronTownsend(260);
 
  int steps = dGas * 1e5;
  avalgrid.SetGrid(0, 4, 400, dGlass, totalThickness-dGlass, steps, -0.05, 0.05, 50);

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
    cCharge = new TCanvas("cCharge", "", 600, 600);
    chargeView = new ViewSignal();
    chargeView->SetCanvas(cCharge);
    chargeView->SetSensor(&sensor);
  }

  // Setting the timer for the running time of the algorithm.
  std::clock_t start = std::clock();
  // Set up Heed.
  TrackHeed track;
  track.SetSensor(&sensor);
  // Set the particle type and momentum [eV/c].
  track.SetParticle("muon");
  track.SetMomentum(1.e9);

  // Simulate a charged-particle track.
  track.NewTrack(0.0, dGlass + 0.999*dGas, 0, 0, 0, -1, 0);
  std::cout<<" track.GetNumberOfClusters() = "<<track.GetClusters().size()<<std::endl;
  // Retrieve the clusters along the track.
  for (const auto &cluster : track.GetClusters()) {
    // Loop over the electrons in the cluster.
    for (const auto &electron : cluster.electrons) {
      // Simulate the electron track
      aval.AvalancheElectron(electron.x, electron.y, electron.z, electron.t,electron.e, 0., 0., 0.);

      int ne = 0, ni = 0, np = 0;
      aval.GetAvalancheSize(ne, ni);
      np = aval.GetNumberOfElectronEndpoints();
      std::cout<<" ne ni np =   "<<ne<<" "<<ni<<" "<<np<<std::endl;

      // Stops calculation after tMaxWindow ns and import electrons in.
      avalgrid.ImportElectronsFromAvalancheMicroscopic(&aval);
    }
  }
  LOG("Switching to grid based methode.");
  avalgrid.AsignLayerIndex(RPC);
  avalgrid.StartGridAvalanche();
  // Stop timer.
  double duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;

  




  /*for (int i=0;i<5;i++)
  {
    std::string strip_labeli = strip_label+std::to_string(i);
    
  
   if (plotSignal&& i==1) {
      // Plot signals
      signalView->PlotSignal(strip_labeli);
      cSignal->Update();
      gSystem->ProcessEvents();

      sensor.ExportSignal(strip_labeli, "Signal");
      // Plot induced charge
      sensor.IntegrateSignal(strip_labeli);
      chargeView->PlotSignal(strip_labeli);
      cCharge->Update();
      gSystem->ProcessEvents();
      // Export induced current data as an csv file.
      sensor.ExportSignal(strip_labeli, "Charge");
    }
    LOG("Script: Total induced charge in strip_labeli:  = " << sensor.GetTotalInducedCharge(strip_labeli)<< " [fC].");

  }*/
  LOG("Script: "<< "Electrons have drifted. It took " << duration << "s to run.");

  if (plotSignal) {
      // Plot signals
      signalView->PlotSignal(label);
      cSignal->Update();
      gSystem->ProcessEvents();

      sensor.ExportSignal(label, "Signal");
      // Plot induced charge
      sensor.IntegrateSignal(label);
      chargeView->PlotSignal(label);
      cCharge->Update();
      gSystem->ProcessEvents();
      // Export induced current data as an csv file.
      sensor.ExportSignal(label, "Charge");
    }
  LOG("Script: Total induced charge in strip_label:  = " << sensor.GetTotalInducedCharge(label)<< " [fC].");


/*  const int nEvents = 1;
  double induced_sum = 0.;
  for (int i=0;i<nEvents;i++)
  {
    sensor.ClearSignal();
    avalgrid.Reset();
    std::cout << i << "/" << nEvents << "\n";

    aval.AvalancheElectron(0, dGlass + 0.999*dGas, 0, 0, 20, 0, -1, 0);

    int ne = 0, ni = 0, np = 0;
    aval.GetAvalancheSize(ne, ni);
    // eGain->Fill(ne);

    np = aval.GetNumberOfElectronEndpoints();
    std::cout<<" ne ni np =   "<<ne<<" "<<ni<<" "<<np<<std::endl;

    avalgrid.ImportElectronsFromAvalancheMicroscopic(&aval);
    
    // Start grid based avalanche calculations starting from where the microsocpic
    // calculations stoped.
    LOG("Switching to grid based methode.");
    avalgrid.AsignLayerIndex(RPC);
    avalgrid.StartGridAvalanche();
    // Stop timer.
    double duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;

    LOG("Script: "
        << "Electrons have drifted. It took " << duration << "s to run.");

    if (plotSignal) {
      // Plot signals
      signalView->PlotSignal(label);
      cSignal->Update();
      gSystem->ProcessEvents();

      sensor.ExportSignal(label, "Signal");
      // Plot induced charge
      sensor.IntegrateSignal(label);
      chargeView->PlotSignal(label);
      cCharge->Update();
      gSystem->ProcessEvents();
      // Export induced current data as an csv file.
      sensor.ExportSignal(label, "Charge");
    }


    sensor.IntegrateSignal(label);
    induced_sum += sensor.GetTotalInducedCharge(label);

  }*/
  //std::cout<<" induced_sum/nEvents : "<<induced_sum/nEvents<<std::endl;
  //sensor.IntegrateSignal("strip_label0");
  //LOG("Script: Total induced charge = " << sensor.GetTotalInducedCharge("strip_label0")<< " [fC].");
  LOG("-------end");

  app.Run(kTRUE);
}
