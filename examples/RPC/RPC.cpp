#include <CLI/CLI.hpp>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <TApplication.h>
#include <TCanvas.h>
#include <TGeoMedium.h>
#include <TH1D.h>
#include <TH1.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TGeoManager.h>
#include <TGeometry.h>
#include <TGeoMaterial.h>

#include <cmath>
#include <ctime>
#include <exception>
#include <iostream>
#include <stdexcept>

#include "Garfield/AvalancheGrid.hh"
#include "Garfield/AvalancheMicroscopic.hh"
#include "Garfield/AvalancheMC.hh"
#include "Garfield/ComponentParallelPlate.hh"
#include "Garfield/FundamentalConstants.hh"
#include "Garfield/GeometrySimple.hh"
#include "Garfield/MediumMagboltz.hh"
#include "Garfield/Plotting.hh"
#include "Garfield/Sensor.hh"
#include "Garfield/SolidBox.hh"
#include "Garfield/TrackHeed.hh"
#include "Garfield/ViewSignal.hh"
#include "Garfield/ViewDrift.hh"
#include "Garfield/GeometryRoot.hh"
#include "Garfield/ComponentAnalyticField.hh"
#include "Garfield/ViewCell.hh"

#include "RPCSim/DefaultFolders.hpp"
#include "RPCSim/new/GasMixture.hpp"
#include "RPCSim/new/WebServer.hpp"

#include <EcoMug.h>

using namespace Garfield;

#include <vector>

spdlog::logger logger("RPCSim", {});


std::vector<std::string> createStrip(Garfield::ComponentAnalyticField& cmp, const std::size_t nbrStrip, const char& direction,const double& position,const double& strip_size, const double& inter_strip,const std::string& name)
{
  std::vector<std::string> ret;
  if(nbrStrip%2==0)
  {
    for(std::size_t i=1; i!=nbrStrip/2+1; ++i)
    {
      std::string name_{fmt::format(name,i)};
      ret.push_back(name_);
      double born1{(i-.5)*inter_strip+(i-1)*strip_size};
      double born2{(i-.5)*inter_strip+(i)*strip_size};
      logger.info("Creating strip {} [{:.2f},{:.2f}] name {}",i,born1,born2,name_);
      cmp.AddStripOnPlaneY(direction,position,born1,born2, name_);
      name_={fmt::format(name,-1.0*i)};
      ret.push_back(name_);
      cmp.AddStripOnPlaneY(direction,position,-1.0*born1,-1.0*born2, name_);
      logger.info("Creating strip {} [{:.2f},{:.2f}] name {}",-1.0*i,-born1,-born2,name_);
    }
  }
  else
  {
    std::string name_={fmt::format(name,0)};
    ret.push_back(name_);
    cmp.AddStripOnPlaneY(direction,position,-strip_size/2.0, strip_size/2.0, name_);
    logger.info("Creating strip {} [{:.2f},{:.2f}] name {}",0,-strip_size/2.0,strip_size/2.0,name_);
    for(std::size_t i=1; i!=(nbrStrip)/2+1; ++i)
    {
      name_=fmt::format(name,i);
      ret.push_back(name_);
      double born1{strip_size/2.0+(i-1)*(strip_size)+i*inter_strip};
      double born2{1.5*strip_size+(i-1)*(strip_size)+i*inter_strip};
      logger.info("Creating strip {} [{:.2f},{:.2f}] name {}",i,born1,born2,name_);
      cmp.AddStripOnPlaneY(direction,position,born1, born2, name_);
      name_=fmt::format(name,-1.0*i);
      ret.push_back(name_);
      logger.info("Creating strip {} [{:.2f},{:.2f}] name {}",-1.0*i,-born1,-born2,name_);
      cmp.AddStripOnPlaneY(direction,position,-1.0*born1,-1.0*born2, name_);
    }
  }
  return ret;
}


int main(int argc, char *argv[]) try
{
  std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  logger=spdlog::logger("RPCSim", {console_sink});
  RPCSim::setEnv();
  CLI::App app("Program to simulate RPCs","RPCSimulator");
  // Command
  int port{8888};
  bool webserver{true};
  std::string gas_file{}; //./c2h2f4_ic4h10_sf6.gas
  std::vector<std::string> gases{"C2H2F4","iC4H10","SF6"};
  std::vector<double> percent{94.7,5,0.3};
  double temperature{20};
  double pressure{1013.25};
  std::array<double,3> magnetic_field{0,0,0};
  double voltage{-6600};
  std::size_t event{1000};
  std::string particle{"muon"};
  double energy{4e9}; //Mean energy of muon at ground form our beloved PDG.
  auto cmd_gas_file=app.add_option("--gasFile", gas_file, "The gas file to use for magboltz")->capture_default_str();
  auto gas_lists=app.add_option("--gases",gases,"Gas names to use")->capture_default_str();
  auto percent_lists=app.add_option("--percent",percent,"Percent of the gas (%)")->capture_default_str();
  app.add_option("--pressure",pressure,"pressure of the gas mixture (°C)")->capture_default_str();
  app.add_option("--temperature",temperature,"temperature of the system (hPa)")->capture_default_str();
  app.add_option("--magnetic_field",magnetic_field,"magnetic field (T)")->capture_default_str();
  app.add_option("--voltage",voltage,"voltage between electrodes (V)")->capture_default_str();
  app.add_option("--events",event,"number of event to process")->capture_default_str();
  app.add_option("--particle",particle,"particle to shoot")->capture_default_str();
  app.add_option("--energy",energy,"energy of the particle (eV)")->capture_default_str();
  cmd_gas_file->excludes(gas_lists,percent_lists);

  // Chamber size
  double size_x{50.};
  double size_z{100.};
  app.add_option("--size_x",size_x,"Width of the chamber")->capture_default_str();
  app.add_option("--size_z",size_z,"Length of the chamber")->capture_default_str();

  // strip config
  double top_strip_size{2}; //cm
  double top_inter_strip_size{0.2}; //cm
  char   top_strip_direction{'z'}; // strip along X direction
  app.add_option("--top_strip_size",top_strip_size,"top (Anode) strip size (cm)")->capture_default_str();
  app.add_option("--top_inter_strip_size",top_inter_strip_size,"top (Anode) inter-strip size (cm)")->capture_default_str();
  app.add_option("--top_strip_direction",top_strip_direction,"direction of the strip ('x' or 'z')")->capture_default_str();

  double bottom_strip_size{-1.0};
  double bottom_inter_strip_size{0.2};
  char bottom_strip_direction{'z'};
  app.add_option("--bottom_strip_size",bottom_strip_size,"bottom (Cathode) strip size (cm)")->capture_default_str();
  app.add_option("--bottom_inter_strip_size",bottom_inter_strip_size,"bottom (Cathode) inter-strip size (cm)")->capture_default_str();
  app.add_option("--bottom_strip_direction",bottom_strip_direction,"direction of the bottom (Cathode) strip ('x' or 'z')")->capture_default_str();

  CLI11_PARSE(app, argc, argv);
  // ROOT Application
  TApplication ROOTApplication("RPC Simulation",0,nullptr);
  RPCSim::WebServer server;
  if(webserver)
  {
    server.bind("http:"+std::to_string(port),false);
  }

  /*******/
  /* Gas */
  /***** */
  // Setup the gas mixture
  RPCSim::GasMixture gas_mixture;
  if(!gas_file.empty())
  {
    logger.info("Using {} file",gas_file);
    gas_mixture.setFile(gas_file);
  }
  else
  {
    logger.info("Using gases ({}) with percents ({})",fmt::join(gases, ","),fmt::join(percent, ","));
    gas_mixture.setMixture(gases,percent);
  }
  logger.info("Temperature {} °C, pressure {}",temperature,pressure);
  gas_mixture.setTemperature(temperature);
  gas_mixture.setPressure(pressure);
  gas_mixture.getMediumMagbolz()->LoadIonMobility(RPCSim::ions_mobility + "/IonMobility_C8Hn+_iC4H10.txt");
  gas_mixture.getMediumMagbolz()->LoadNegativeIonMobility(RPCSim::ions_mobility + "/IonMobility_SF6-_SF6.txt");
  gas_mixture.initialise(true);

  /************/
  /* Geometry */
  /************/
  // NOTICE : I use a & * trick to signify this pointers are handled by ROOT (ROOT please understand pointer are EVIL !!!)
  // The dimensions

  const double gasGapThickness{0.1}; //1mm
  const double topGlassThickness{0.11}; //1.1mm
  const double bottomGlassThickness{0.11}; //1.1mm
  const double graphiteThickness{0.01}; //100um
  const double topMylarThickness{0.01}; //100um
  const double bottomMylarThickness{0.01}; //100um
  // The manager
  TGeoManager RPCGeometry("RPCGeometry", "RPC geometry");
  // The element table
  TGeoElementTable& table=*RPCGeometry.GetElementTable();
  // Create the universe with vacuum
  TGeoVolume& universe = *RPCGeometry.MakeBox("RPC", new TGeoMedium("Vacuum",1,new TGeoMaterial("Vacuum", 0, 0, 0)), size_x/2.0, 100/2.0, size_z/2.0);
  RPCGeometry.SetTopVolume(&universe);
  // Create the gas gap with center of gravity at (0,0,0)
  TGeoMedium gas("Gas",1,new TGeoMaterial("GasMixture",gas_mixture.getMediumMagbolz()->GetAtomicWeight(),gas_mixture.getMediumMagbolz()->GetAtomicNumber(),gas_mixture.getMediumMagbolz()->GetMassDensity(), TGeoMaterial::EGeoMaterialState::kMatStateGas, temperature, pressure));
  TGeoVolume& gas_gap = *RPCGeometry.MakeBox("Gas gap", &gas, size_x/2.0, gasGapThickness/2.0, size_z/2.0);
  universe.AddNode(&gas_gap,1,new TGeoTranslation(0,0,0));
  gas_gap.SetFillColor(kBlue);
  gas_gap.SetTransparency(95);
  // Variable to place the center of gravity of other part in the right place
  double to_up{gasGapThickness/2.0+topGlassThickness/2.0};
  double to_down{gasGapThickness/2.0+bottomGlassThickness/2.0};

  // Create glass material
  // https://www.fe.infn.it/u/paterno/Geant4_tutorial/slides_further/Geometry/G4_Nist_Materials.pdf
  TGeoMixture glass("Glass",4,2.4);
  glass.AddElement(table.GetElement(8),0.4598);
  glass.AddElement(table.GetElement(11),0.0964411);
  glass.AddElement(table.GetElement(14),0.336553);
  glass.AddElement(table.GetElement(20),0.107205);
  TGeoMedium glass_medium("ElectrodeMedium",1,&glass);
  // Create Top Glass
  TGeoVolume& top = *RPCGeometry.MakeBox("Top Glass", &glass_medium, size_x/2.0, topGlassThickness/2.0, size_z/2.0);
  top.SetFillColor(kRed);
  top.SetTransparency(0);
  universe.AddNode(&top,1,new TGeoTranslation(0,to_up,0));
  // Create Bottom glass
  TGeoVolume& bottom = *RPCGeometry.MakeBox("Bottom Glass", &glass_medium, size_x/2.0, bottomGlassThickness/2.0, size_z/2.0);
  bottom.SetFillColor(kRed);
  bottom.SetTransparency(0);
  universe.AddNode(&bottom,1,new TGeoTranslation(0,-to_down,0));
  // Add Graphite
  // Create graphite material
  TGeoMixture graphite("Graphite",1,1.7);
  graphite.AddElement(table.GetElement(6),1.0);
  TGeoMedium graphite_medium("Graphite",1,&graphite);
  to_up+=graphiteThickness;
  to_down+=graphiteThickness;
  TGeoVolume& top_graphite_layer = *RPCGeometry.MakeBox("Top graphite layer", &graphite_medium, size_x/2.0, graphiteThickness/2.0, size_z/2.0);
  bottom.SetFillColor(kBlack);
  bottom.SetTransparency(0);
  universe.AddNode(&top_graphite_layer,1,new TGeoTranslation(0,to_up,0));
  TGeoVolume& bottom_graphite_layer = *RPCGeometry.MakeBox("Bottom graphite layer", &graphite_medium, size_x/2.0, graphiteThickness/2.0, size_z/2.0);
  bottom.SetFillColor(kBlack);
  bottom.SetTransparency(0);
  universe.AddNode(&bottom_graphite_layer,1,new TGeoTranslation(0,-to_down,0));
  // Applying voltage position //
  double position_voltage_top=to_up+graphiteThickness/2.0;
  double position_voltage_bottom=-to_down-graphiteThickness/2.0;
  // Add mylar
  TGeoMixture mylar("Mylar",3,1.4);
  mylar.AddElement(table.GetElement(1),0.041959);
  mylar.AddElement(table.GetElement(6),0.625016);
  mylar.AddElement(table.GetElement(8),0.333025);
  TGeoMedium mylar_medium("Mylar",1,&mylar);
  to_up+=topMylarThickness;
  to_down+=bottomMylarThickness;
  TGeoVolume& top_mylar_layer = *RPCGeometry.MakeBox("Top mylar layer", &mylar_medium, size_x/2.0, topMylarThickness/2.0, size_z/2.0);
  bottom.SetFillColor(kBlue);
  bottom.SetTransparency(80);
  universe.AddNode(&top_mylar_layer,1,new TGeoTranslation(0,to_up,0));
  TGeoVolume& bottom_mylar_layer = *RPCGeometry.MakeBox("Bottom mylar layer", &mylar_medium, size_x/2.0, bottomMylarThickness/2.0, size_z/2.0);
  bottom.SetFillColor(kBlue);
  bottom.SetTransparency(80);
  universe.AddNode(&bottom_mylar_layer,1,new TGeoTranslation(0,-to_down,0));
  // Reading position (strip position)
  double top_reading_position= to_up+topMylarThickness/2.0;
  double bottom_reading_position=-to_down-bottomMylarThickness/2.0;
  // Seems garfield need to have the gas gap as top volume !??!!??!??!
  RPCGeometry.SetTopVolume(&gas_gap);
  RPCGeometry.CloseGeometry();
  RPCGeometry.SetVisLevel(4);
  server().Register("/Geometry", &RPCGeometry);
  // Create the Garfield geometry.
  Garfield::GeometryRoot* geo = new GeometryRoot();
  // Pass the pointer to the TGeoManager.
  geo->SetGeometry(&RPCGeometry);
  // Associate the ROOT medium with the Garfield medium.
  geo->SetMedium("GasMixture", gas_mixture.getMediumMagbolz());



  const bool debug = true;
  constexpr bool plotSignal = true;

  Garfield::ComponentAnalyticField cmp;
  cmp.SetGeometry(geo);
  cmp.SetMagneticField(magnetic_field[0], magnetic_field[1], magnetic_field[2]);
  //cmp.EnableDebugging();
  
  cmp.AddPlaneY(position_voltage_top,0,"Anode");
  cmp.AddPlaneY(position_voltage_bottom,-6200,"Cathode");

  std::size_t number_strip_top_side{0};
  std::size_t number_strip_bottom_side{0};
  std::vector<std::string> top_names;
  std::vector<std::string> bottom_names;
  if(top_strip_size>=0)
  {
    if(top_strip_direction=='z')
    {
      number_strip_top_side=(size_x-top_inter_strip_size)/(top_inter_strip_size+top_strip_size);
    }
    else if(top_strip_direction=='x')
    {
      number_strip_top_side=(size_z-top_inter_strip_size)/(top_inter_strip_size+top_strip_size);
    }
    logger.info("Top side (Anode) will have {} strips with {} cm thickness {} cm inter-strip along {} direction",number_strip_top_side,top_strip_size,top_inter_strip_size,top_strip_direction);
    top_names=createStrip(cmp, number_strip_top_side, top_strip_direction,top_reading_position,top_strip_size, top_inter_strip_size,"top_strip_{}");
  }
  if(bottom_strip_size>=0)
  {
    if(bottom_strip_direction=='z')
    {
      number_strip_bottom_side=(size_x-bottom_inter_strip_size)/(bottom_inter_strip_size+bottom_strip_size);
    }
    else if(bottom_strip_direction=='x')
    {
      number_strip_bottom_side=(size_z-bottom_inter_strip_size)/(bottom_inter_strip_size+bottom_strip_size);
    }
    logger.info("Bottom side (Cathode) will have {} strips with {} cm thickness {} cm inter-strip along {} direction",number_strip_bottom_side,bottom_strip_size,bottom_inter_strip_size,bottom_strip_direction);
    bottom_names=createStrip(cmp, number_strip_bottom_side, bottom_strip_direction,bottom_reading_position,bottom_strip_size, bottom_inter_strip_size,"bottom_strip_{}");
  }
  // Create a viewer.
  Garfield::ViewCell view;
  // Set the pointer to the component.
  view.SetComponent(&cmp);
  // Make a two-dimensional plot of the cell layout.
  view.Plot3d();


  Garfield::Shaper shaper(1,1e-8,50,"bipolar");
  // Create the sensor.
  Garfield::Sensor sensor;
  sensor.AddComponent(&cmp);
  // Request signal calculation for the electrode named "s",
  // using the weighting field provided by the Component object cmp.
  for(std::size_t i=0;i!=top_names.size();++i)
  {
    sensor.AddElectrode(&cmp, top_names[i]);
  }
  for(std::size_t i=0;i!=bottom_names.size();++i)
  {
    sensor.AddElectrode(&cmp, bottom_names[i]);
  }
  sensor.SetTransferFunction(shaper);

  // Set the time bins.
  const unsigned int nTimeBins = 220;
  const double tmin = -1.;
  const double tmax = 10;
  const double tstep = (tmax - tmin) / nTimeBins;
  sensor.SetTimeWindow(tmin, tstep, nTimeBins);

  // Create the AvalancheMicroscopic.
  AvalancheMicroscopic aval;
  aval.SetSensor(&sensor);
  aval.EnableSignalCalculation();
  //aval.EnableInducedChargeCalculation();
  aval.UseWeightingPotential();
  aval.EnablePhotonTransport();
  aval.EnableMagneticField();

  // Set time window where the calculations will be done microscopically.
  //const double tMaxWindow = 0.1;
  //aval.SetTimeWindow(0., tMaxWindow);

  // Create the AvalancheGrid for grid-based avalanche calculations that are
  // suited for constant drift fields. This class will take over the
  // calculations of the microscopic class after the set time-window.
  //AvalancheGrid avalgrid;
  //avalgrid.SetSensor(&sensor);

  //int steps = totalThickness * 1e4;
  //avalgrid.SetGrid(-0.05, 0.05, 5, 0.0, totalThickness, steps, -0.05, 0.05, 5);

  // Preparing the plotting of the induced charge and signal of the electrode
  // readout.
  ViewSignal *signalView = nullptr;
  TCanvas *cSignal = nullptr;
  if (plotSignal) {
    cSignal = new TCanvas("cSignal", "", 600, 600);
    signalView = new ViewSignal();
    signalView->SetCanvas(cSignal);
    signalView->SetSensor(&sensor);
    server.attach("/",*cSignal);
    server().SetItemField("/","_monitoring","10");
    server().SetItemField("/","_layout","horiz2");
    server().SetItemField("/","_drawitem","[cSignal,cCharge]");
    server().SetItemField("/","_drawopt","AL");
  }

  ViewSignal *chargeView = nullptr;
  TCanvas *cCharge = nullptr;

  if (plotSignal) {
    cCharge = new TCanvas("cCharge", "", 600, 600);
    chargeView = new ViewSignal();
    chargeView->SetCanvas(cCharge);
    chargeView->SetSensor(&sensor);
    server.attach("/",*cCharge);
  }


 ViewSignal *convoView = nullptr;
  TCanvas *cConvo = nullptr;

  if (plotSignal) {
    cConvo = new TCanvas("cConvo", "", 600, 600);
    convoView = new ViewSignal();
    convoView->SetCanvas(cConvo);
    convoView->SetSensor(&sensor);
    server.attach("/",*cConvo);
  }

  // Set up Heed.
  TrackHeed track;
  track.SetSensor(&sensor);

  // Generate muons
  EcoMug gen;
  gen.SetSkySize({{size_x, size_z}}); // x and y size of the plane
  // (x,y,z) position of the center of the plane
  gen.SetSkyCenterPosition({{0,0, gasGapThickness/2.}});
  // Track photons
  TrackHeed photon_tracker(&sensor);
  photon_tracker.EnableElectricField();
  photon_tracker.EnableMagneticField();

  // For the ions
  AvalancheMC drift_ions;
  drift_ions.SetSensor(&sensor);
  drift_ions.EnableSignalCalculation();
  drift_ions.UseWeightingPotential();
  drift_ions.SetIonSignalScalingFactor(1.);

  //Garfield::ViewDrift driftview;
  //TCanvas* drift= new TCanvas("CD", "", 600, 600);
  //const bool twod = false;
  //const bool axis = true;
  //driftview.SetCanvas(drift);
  //driftview.Plot(twod,axis);
  //track.EnablePlotting(&driftview);//plot the track of electron
  //photon_tracker.EnablePlotting(&driftview);//plot the track of proton
  //drift_ions.EnablePlotting(&driftview);
  //server.attach("/",*drift);

  TCanvas canvas("Cluster","Cluster",1900,1000);
  canvas.Divide(5,2);
  TH1D cluster_number("cluster_number","Number of clusters;# clusters;",100,0,100);
  TH1D electron_number("electron_number","Number of electrons;# e-;",20,0,20);
  TH1D ion_number("ion_number","Number of ions;# ions;",20,0,20);
  TH1D photon_number("photon_number","Number of photons;# #gamma;",20,0,20);
  TH1D pos_x("delta_x","#Delta(x);#Delta(x);",200,-10,10);
  TH1D pos_y("delta_y","#Delta(y);#Delta(y);",200,-10,10);
  TH1D pos_z("delta_z","#Delta(z);#Delta(z);",200,-10,10);
  TH1D time("delta_t","#Delta(t);#Delta(t);",50,0,0.5);
  TH1D cluster_energy("cluster_energy","Energy of cluster;E(eV);# clusters",180,0,180);
  TH1D total_energy("energy_of_clusters","Total energy of the clusters;E(eV);# events",200,0,200);
  server.attach("/",canvas);
  canvas.cd(1);
  cluster_number.Draw();
  canvas.cd(2);
  electron_number.Draw();
  canvas.cd(3);
  ion_number.Draw();
  canvas.cd(4);
  photon_number.Draw();
  canvas.cd(5);
  pos_x.Draw();
  canvas.cd(6);
  pos_y.Draw();
  canvas.cd(7);
  pos_z.Draw();
  canvas.cd(8);
  time.Draw();
  canvas.cd(9);
  cluster_energy.Draw();
  canvas.cd(10);
  total_energy.Draw();


  TCanvas secondaries("secondaries","secondaries",1900,1000);
  TH1D secondaries_electrons("","",200,0,200);
  secondaries.cd();
  secondaries_electrons.Draw();
  aval.EnableSecondaryEnergyHistogramming(&secondaries_electrons);

  TCanvas secondaries2("secondaries2","secondaries2",1900,1000);
  TH1D secondaries_electrons2("","",200,0,200);
  secondaries2.cd();
  secondaries_electrons2.Draw();
  aval.EnableElectronEnergyHistogramming(&secondaries_electrons2);


  for(std::size_t i=0; i!=event;++i)
  {
    logger.info("Generating event {}/{}",i+1,event);
    sensor.ClearSignal();
    // Setting the timer for the running time of the algorithm.
    std::clock_t start = std::clock();
    // Simulate a charged-particle track.
    // Set the particle type and momentum [eV/c].
    gen.Generate();
    std::array<double, 3> muon_position = gen.GetGenerationPosition();
    double muon_p = gen.GetGenerationMomentum();
    double muon_theta = gen.GetGenerationTheta();
    double muon_phi = gen.GetGenerationPhi();
    double muon_charge = gen.GetCharge();
    logger.info("Generating {} at position {} {} {} p {} theta {} phi {}",muon_charge,muon_position[0],muon_position[1],muon_position[2],muon_p,muon_theta,muon_phi);
    logger.info("Direction {} {} {}",std::sin(muon_theta)*std::cos(muon_phi),std::cos(muon_theta),-1.0*std::sin(muon_theta)*std::sin(muon_phi));
    if(particle<0)track.SetParticle("muon");
    else track.SetParticle("mu+");
    track.SetMomentum(muon_p*1e9);
    double track_x{muon_position[0]};
    double track_y{gasGapThickness/2.};
    double track_z{-1.0*muon_position[1]};
    double track_t{0.};
    double track_dx{std::sin(muon_theta)*std::cos(muon_theta)};
    double track_dy{std::cos(muon_theta)};
    double track_dz{-std::sin(muon_theta)*std::sin(muon_theta)};
    track.NewTrack(track_x, track_y,track_z, 0., track_dx,track_dy,track_dz);
    logger.info("Generating a {} at [{:.2f},{:.2f},{:.2f}] cm with momentum {:.2f} eV direction =[{:.2f},{:.2f},{:.2f}]","mu",track_x,track_y,track_z,track.GetMomentum(),track_dx,track_dy,track_dz);
    track.DisableDeltaElectronTransport();
    // Retrieve the clusters along the track.
    canvas.cd(1);
    cluster_number.Fill(track.GetClusters().size(),1.0);
    cluster_number.Draw();
    canvas.Update();
    double total_clusters_energy{0.};
    for (const auto &cluster : track.GetClusters())
    {
      canvas.cd(2);
      electron_number.Fill(cluster.electrons.size(),1.0);
      electron_number.Draw();
      canvas.cd(3);
      ion_number.Fill(cluster.ions.size(),1.0);
      ion_number.Draw();
      canvas.cd(4);
      photon_number.Fill(cluster.photons.size(),1.0);
      photon_number.Draw();
      canvas.cd(5);
      pos_x.Fill(cluster.x-track_x,1.0);
      pos_x.Draw();
      canvas.cd(6);
      pos_y.Fill(cluster.y-track_y,1.0);
      pos_y.Draw();
      canvas.cd(7);
      pos_z.Fill(cluster.z-track_z,1.0);
      pos_z.Draw();
      canvas.cd(8);
      time.Fill(cluster.t-track_t,1.0);
      time.Draw();
      canvas.cd(9);
      cluster_energy.Fill(cluster.energy,1.0);
      cluster_energy.Draw();
      canvas.Update();
      total_clusters_energy+=cluster.energy;
      // Loop over the electrons in the cluster.
      for(const auto &ion : cluster.ions )
      {
        gSystem->ProcessEvents();
        if(sensor.IsInside(ion.x, ion.y, ion.z) && sensor.IsInArea(ion.x, ion.y, ion.z)) drift_ions.DriftIon(ion.x, ion.y,ion.z, ion.t); 
      }
      // Loop over the photons in the cluster.
      for(const auto &photon: cluster.photons)
      {
        gSystem->ProcessEvents();
        int electrons{0};
        int ions{0};
        int fluorescent{0};
        photon_tracker.TransportPhoton(photon.x,photon.y,photon.z,photon.t,photon.e,photon.dx,photon.dy,photon.dz, electrons,ions,fluorescent);
        for (const auto &cluster : photon_tracker.GetClusters())
        {
          std::cout<<"*** "<<electrons<<" "<<ions<<" "<<fluorescent<<" "<<cluster.electrons.size()<<"  "<<cluster.ions.size()<<" "<<cluster.photons.size()<<std::endl;
          for(const auto &ion : cluster.ions )
          {
            if(sensor.IsInside(ion.x, ion.y, ion.z) && sensor.IsInArea(ion.x, ion.y, ion.z)) drift_ions.DriftIon(ion.x, ion.y,ion.z, ion.t); 
          }
          for(const auto &electron : cluster.electrons)
          {
            // Simulate the electron track
            aval.AvalancheElectron(electron.x, electron.y, electron.z, electron.t,electron.e, electron.dx, electron.dy, electron.dz);

            for(std::size_t ion = 0; ion < aval.GetNumberOfElectronEndpoints(); ++ion)
            {
              double xe1, ye1, ze1, te1, e1;
              double xe2, ye2, ze2, te2, e2;
              int status;
              aval.GetElectronEndpoint(ion, xe1, ye1, ze1, te1, e1, xe2, ye2, ze2, te2, e2, status);
              if(status!=StatusLeftDriftMedium)drift_ions.DriftIon(xe1, ye1, ze1, te1); 
              if(status == -7) /*if(box.IsInside(xe2,ye2,ze2,true))*/ drift_ions.DriftNegativeIon(xe2, ye2, ze2, te2);
              else continue;
            }
          }
        }
      }
      for(const auto &electron: cluster.electrons)
      {
        
        secondaries.cd();
        secondaries_electrons.Draw();
        secondaries.Update();
        secondaries2.cd();
        secondaries_electrons2.Draw();
        secondaries2.Update();
        gSystem->ProcessEvents();
        // Simulate the electron track
        aval.AvalancheElectron(electron.x, electron.y, electron.z, electron.t,electron.e, electron.dx, electron.dy, electron.dz);
        for(std::size_t ion = 0; ion < aval.GetNumberOfElectronEndpoints(); ++ion)
        {
          gSystem->ProcessEvents();
          double xe1, ye1, ze1, te1, e1;
          double xe2, ye2, ze2, te2, e2;
          int status;
          aval.GetElectronEndpoint(ion, xe1, ye1, ze1, te1, e1, xe2, ye2, ze2, te2, e2, status);
          if(status!=StatusLeftDriftMedium)drift_ions.DriftIon(xe1, ye1, ze1, te1); 
          if(status == -7) /*if(box.IsInside(xe2,ye2,ze2,true))*/ drift_ions.DriftNegativeIon(xe2, ye2, ze2, te2);
          else continue;
        }
      }
      canvas.cd(10);
      total_energy.Fill(total_clusters_energy,1.0);
      total_energy.Draw();
      canvas.Update();
    }

    // Start grid based avalanche calculations starting from where the microsocpic
    // calculations stoped.
    //std::cout<<"Switching to grid based methode."<<std::endl;
    //avalgrid.AsignLayerIndex(RPC);
    //avalgrid.StartGridAvalanche();
    // Stop timer.
    double duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;

    std::cout<<"Script: Electrons have drifted. It took " << duration << "s to run."<<std::endl;

    if (plotSignal) {
      for(std::size_t i=0;i!=top_names.size();++i)
      {
        // Plot signals
        signalView->PlotSignal(top_names[i],"tei","tei","tei",false);
        cSignal->SetTitle(fmt::format("Signal event {}/{}",i+1,event).c_str());
        cSignal->Update();
        gSystem->ProcessEvents();
        //sensor.ConvoluteSignal(top_names[i]);
        //convoView->PlotSignal(top_names[i]);
        //cConvo->SetTitle(fmt::format("Convoluted signal event {}/{}",i+1,event).c_str());
        //cConvo->Update();
        sensor.IntegrateSignal(top_names[i]);
        chargeView->PlotSignal(top_names[i]);
        cCharge->SetTitle(fmt::format("Integrated charge event {}/{}",i+1,event).c_str());
        cCharge->Update();
        logger.info("Script: Total induced charge {}={} fC",top_names[i],sensor.GetTotalInducedCharge(top_names[i]));
      }
      for(std::size_t i=0;i!=bottom_names.size();++i)
      {
        // Plot signals
        signalView->PlotSignal(bottom_names[i],"tei","tei","tei",false);
        cSignal->SetTitle(fmt::format("Signal event {}/{}",i+1,event).c_str());
        cSignal->Update();
        gSystem->ProcessEvents();
        //sensor.ConvoluteSignal(bottom_names[i]);
        //convoView->PlotSignal(bottom_names[i]);
        //cConvo->SetTitle(fmt::format("Convoluted signal event {}/{}",i+1,event).c_str());
        //cConvo->Update();
        sensor.IntegrateSignal(bottom_names[i]);
        chargeView->PlotSignal(bottom_names[i]);
        cCharge->SetTitle(fmt::format("Integrated charge event {}/{}",i+1,event).c_str());
        cCharge->Update();
        logger.info("Script: Total induced charge {}={} fC",bottom_names[i],sensor.GetTotalInducedCharge(bottom_names[i]));
      }
    }
    
  }
  ROOTApplication.Run(kTRUE);
}
catch(const std::exception& exception)
{
  logger.critical(exception.what());
}
catch(...)
{
  logger.critical("Unknow error");
}