#include <Rtypes.h>
#include <TApplication.h>
#include <TGeoManager.h>
#include <TROOT.h>
#include <TSystem.h>
#include <chrono>
#include <string>


#include "Garfield/GeometryRoot.hh"
#include "Garfield/AvalancheGrid.hh"
#include "Garfield/AvalancheMC.hh"
#include "Garfield/Random.hh"
#include "Garfield/ComponentAnalyticField.hh"
#include "Garfield/Plotting.hh"
#include "Garfield/SolidBox.hh"
#include "Garfield/TrackHeed.hh"
#include "Garfield/ViewSignal.hh"
#include "Garfield/ViewField.hh"
#include "Garfield/ViewGeometry.hh"
#include "Garfield/ViewCell.hh"
#include "Garfield/ViewDrift.hh"


#include "spdlog/spdlog.h"
#include "spdlog/sinks/udp_sink.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "RPCSim/SignalPlotter.hpp"
#include "RPCSim/Webserver.hpp"

#include <TGraph2D.h>
#include <TH2D.h>


#include "RPCSim/DefaultFolders.hpp"
#include "RPCSim/Exceptions.hpp"
#include "RPCSim/Setup.hpp"
#include "RPCSim/RPCGeometry.hpp"
#include "RPCSim/PCBGeometry.hpp"


using namespace Garfield;


auto timeFuncInvocation = 
    [](auto&& func, auto&&... params) {
        // get time before function invocation
        const auto& start = std::chrono::high_resolution_clock::now();
        // function invocation using perfect forwarding
        std::forward<decltype(func)>(func)(std::forward<decltype(params)>(params)...);
        // get time after function invocation
        const auto& stop = std::chrono::high_resolution_clock::now();
        return stop - start;
     };

#include <CLI/CLI.hpp>

int main(int argc, char *argv[]) try
{
  std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  spdlog::logger logger("RPCSim", {console_sink});
  logger.set_level(spdlog::level::debug);
  // Setup all this mess
  RPCSim::setEnv();
  // Options
  CLI::App app;
  app.description("Program to simulate RPCs");
  app.name("RPCSimulator");
  std::size_t nbr_events{1000};
  std::string save_file{};
  bool start_webserver{false};
  int port{8888};
  try
  {
    app.add_option("-e,--events", nbr_events, "Number of events to simulate")->capture_default_str();
    app.add_option("-s,--save_file",save_file,"File to save the data (.root)");
    auto port_arg=app.add_option("-p,--port", port, "port to listen to")->capture_default_str();
    auto server = app.add_flag("--server",start_webserver,"Start the webserver");
    server->needs(port_arg);
    app.parse(argc,argv);
  }
  catch(const CLI::ParseError &e)
  {
    return app.exit(e);
  }
  catch(...)
  {
    throw;
  }

  RPCSim::Webserver server("http:"+std::to_string(port),true);

// Geometry
TGeoManager* geoman = new TGeoManager("RPC_Geometry", "RPC geometry");
TGeoMedium* medVacuum = new TGeoMedium("Vacuum", 1,new TGeoMaterial("Vacuum", 0, 0, 0));
TGeoVolume* universe = geoman->MakeBox("RPC", medVacuum, 50, 0.05, 50.);
geoman->SetTopVolume(universe);
TGeoVolume* gas_gap = geoman->MakeBox("Gas gap", medVacuum, 100/2.0, 0.1/2.0, 100/2);
gas_gap->SetFillColor(kBlue);
universe->AddNode(gas_gap,1,new TGeoTranslation(0,0,0));
TGeoVolume* top = geoman->MakeBox("Top graphite", medVacuum, 100/2.0, 0.1/2.0, 100/2);
top->SetFillColor(kRed);
universe->AddNode(top,1,new TGeoTranslation(0,0.1,0));
TGeoVolume* bottom = geoman->MakeBox("Bottom graphite", medVacuum, 100/2.0, 0.1/2.0, 100/2);
bottom->SetFillColor(kRed);
universe->AddNode(bottom,1,new TGeoTranslation(0,-0.1,0));


geoman->CloseGeometry();
geoman->SetVisLevel(4);
server().Register("/Geometry", geoman);
//universe->Draw();
// Create the Garfield medium.
MediumMagboltz* gas = new MediumMagboltz();
  gas->LoadGasFile(RPCSim::data_folder+"/examples/c2h2f4_94-7_iso_5_sf6_0-3_bis.gas");
  gas->LoadIonMobility(RPCSim::garfield_install + "/share/Garfield/Data/IonMobility_C8Hn+_iC4H10.txt");
  gas->LoadNegativeIonMobility(RPCSim::garfield_install+"/share/Garfield/Data/IonMobility_SF6-_SF6.txt");
  gas->Initialise();
// Create the Garfield geometry.
GeometryRoot* geo = new GeometryRoot();
// Pass the pointer to the TGeoManager.
geo->SetGeometry(geoman);
// Associate the ROOT medium with the Garfield medium.
geo->SetMedium("Vacuum", gas);



  Garfield::ComponentAnalyticField cmp;
  cmp.SetGeometry(geo);
  cmp.SetMagneticField(0, 0, 0);
  //cmp.EnableDebugging();
  
  cmp.AddPlaneY(-0.15,0,"Anode");
  cmp.AddPlaneY(0.15,-6200,"Cathode");
  constexpr double pitch = 4;
  constexpr double extar=0.1;
  const std::size_t nbr_strips{20};
  for(std::size_t i=0;i!=1;++i)
  {
    cmp.AddStripOnPlaneY('z', -0.5, i*pitch+(i*extar), (i+1)*pitch+(i*extar), "top_side_strip_"+std::to_string(i));
    cmp.AddStripOnPlaneY('z', 0.5, i*pitch+(i*extar), (i+1)*pitch+(i*extar), "bottom_side_strip_"+std::to_string(i)); // For now they are on the same direction //TODO CHECK THIS
  }

 const bool debug = true;
  constexpr bool plotSignal = true;

  Garfield::Sensor sensor;
  //sensor.EnableDebugging();
  // Calculate the electric field using the Component object cmp.
  sensor.AddComponent(&cmp);
  // Request signal calculation for the electrode named "s",
  // using the weighting field provided by the Component object cmp.
  for(std::size_t i=0;i!=1;++i)
  {
    sensor.AddElectrode(&cmp, "top_side_strip_"+std::to_string(i));
    sensor.AddElectrode(&cmp, "bottom_side_strip_"+std::to_string(i));
  }
  // Set the time bins.
  const unsigned int nTimeBins = 1000;
  const double tmin = 0.;
  const double tmax = 100;
  const double tstep = (tmax - tmin) / nTimeBins;
  sensor.SetTimeWindow(tmin, tstep, nTimeBins);

  // Create the AvalancheMicroscopic.
  AvalancheMicroscopic aval;
 // aval.EnableDebugging();
  aval.SetSensor(&sensor);
  aval.EnableSignalCalculation();
  aval.UseWeightingPotential();

  // Set time window where the calculations will be done microscopically.
  const double tMaxWindow = 2000;
  aval.SetTimeWindow(0., tMaxWindow);
  // For ions
  Garfield::AvalancheMC aval_MC;
  aval_MC.SetSensor(&sensor);

  // Set up Heed.
  TrackHeed track;
  track.EnableDebugging();
  //track.EnableDeltaElectronTransport();
  track.SetSensor(&sensor);
  // Set the particle type and momentum [eV/c].
  track.SetParticle("muon");
  track.SetMomentum(200.e6);
  track.DisableDeltaElectronTransport();




  RPCSim::SignalPlotter plotter_bottom(std::string("Bottom_strips_signal"),std::string("Signal from the bottom strips"));
  plotter_bottom.setSensor(&sensor);
  plotter_bottom.attach(server);
  RPCSim::SignalPlotter plotter_top(std::string("Top_strips_signal"),std::string("Signal from the top strips"));
  plotter_top.setSensor(&sensor);
  plotter_top.attach(server);
  TCanvas general("gl_general","General variables",0,0,1820,1000);
  server().Register("/Monitoring", &general);
  server().SetItemField("/","_monitoring","100");
  server().SetItemField("/","_layout","simple");
  server().SetItemField("/","_drawitem","gl_general");
  server().SetItemField("/","_drawopt","AL");
  general.Divide(3,3);
  general.cd(1);
  TH1D nbr_cluster("gl_number_clusters","# clusters",100,0,101);
  nbr_cluster.Draw();
  general.Update();
  general.cd(2);
  TH1D cluster_energy("gl_cluster_energy","cluster_energy",40000,0,40001);
  cluster_energy.Draw();
  general.Update();
  general.cd(3);
  TH1D cluster_dt("gl_cluster_dt","cluster time #Delta(t)",10000,0,1);
  cluster_dt.Draw();
  general.Update();
  general.cd(4);
  TH1D cluster_position_x("gl_cluster_position_x","cluster position #Delta(x)",10000,-5,5);
  cluster_position_x.Draw();
  general.Update();
  general.cd(5);
  TH1D cluster_position_y("gl_cluster_position_y","cluster position #Delta(y)",10000,-5,5);
  cluster_position_y.Draw();
  general.Update();
  general.cd(6);
  TH1D cluster_position_z("gl_cluster_position_z","cluster position #Delta(z)",10000,-5,5);
  cluster_position_z.Draw();
  general.Update();
  general.cd(7);
  TH1D nbr_photons("gl_nbr_photons","# photons/event",20,0,20);
  nbr_photons.Draw();
  general.Update();
  general.cd(8);
  TH1D nbr_ions("gl_nbr_ions","# ions/event",20,0,20);
  nbr_ions.Draw();
  general.Update();
  general.cd(9);
  TH1D nbr_electrons("gl_nbr_electrons","# electrons/event",20,0,20);
  nbr_electrons.Draw();
  general.Update();

  //TGraph2D cluster_position;


  Garfield::ViewDrift primary;
  primary.EnableDebugging();
  server().Register("/Primaries",primary.GetCanvas());
  primary.GetCanvas()->Update();
  //track.EnablePlotting(&primary);

  
  Garfield::ViewDrift conduction;
  server().Register("/Conduction",conduction.GetCanvas());
  conduction.GetCanvas()->Update();
  //aval.EnablePlotting(&conduction,100);
  //aval_MC.EnablePlotting(&conduction);

  for(std::size_t event=0; event!=nbr_events; ++event)
  {
      logger.info("Event number {}/{}",event+1,nbr_events);
      sensor.ClearSignal();
      // Setting the timer for the running time of the algorithm.
      std::clock_t start = std::clock();
      double track_x{25.0};
      double track_y{0.05};
      double track_z{5.0};
      double track_t{0};
      double track_dx{0.};
      double track_dy{-1.};
      double track_dz{0.};
      logger.info("Shooting an at position=({:+.2e},{:+.2e},{:+.2e}) cm, direction({:+.2e},{:+.2e},{:+.2e}) cm, time={:+.2e} ns, energy={:.2e} eV",track_x,track_y,track_z,track_dx,track_dy,track_dz,track_t,track.GetEnergy());
      track.NewTrack(track_x,track_y,track_z,track_t, 0., -1.0, 0.);
      track.DisableDeltaElectronTransport();
      primary.Plot();
      primary.GetCanvas()->Update();
      // Retrieve the clusters along the track.
      general.cd(1);
      nbr_cluster.Fill(track.GetClusters().size(),1.0);
      nbr_cluster.Draw();
      double energy{0.};
      logger.info("{} cluster(s) has been created, cluster density={:.3e} cl/cm-3, stopping power={:.3e} eV",track.GetClusters().size(),track.GetClusterDensity(),track.GetStoppingPower());
      for(const auto &cluster : track.GetClusters())
      {
        static std::size_t cluster_nr{0};
        logger.info("cluster {}/{} at position=({:+.2e},{:+.2e},{:+.2e}) from track, created {:+.2e}ns after track, energy={:+.2e}",cluster_nr+1,track.GetClusters().size(), cluster.x-track_x,cluster.y-track_y,cluster.z-track_z,cluster.t-track_t,cluster.energy);
        energy+=cluster.energy;
        general.cd(3);
        cluster_dt.Fill(cluster.t-track_t);
        cluster_dt.Draw();
        general.cd(4);
        cluster_position_x.Fill(cluster.x-track_x);
        cluster_position_x.Draw();
        general.cd(5);
        cluster_position_y.Fill(cluster.y-track_y);
        cluster_position_y.Draw();
        general.cd(6);
        cluster_position_z.Fill(cluster.z-track_z);
        cluster_position_z.Draw();
        general.cd(7);
        nbr_photons.Fill(cluster.photons.size(),1.0);
        nbr_photons.Draw();
        general.cd(8);
        nbr_ions.Fill(cluster.ions.size(),1.0);
        nbr_ions.Draw();
        general.cd(9);
        nbr_electrons.Fill(cluster.electrons.size(),1.0);
        nbr_electrons.Draw();
        general.Update();
      logger.warn("Now the photons !!!");
      for(std::size_t p=0;p!=cluster.photons.size();++p)
      {
        int ne{0};
        int ni{0};
        int np{0};// CHECK I need to drift them
        track.TransportPhoton(cluster.photons[p].x, cluster.photons[p].y, cluster.photons[p].z, cluster.photons[p].t, cluster.photons[p].e,Garfield::RndmUniform(),Garfield::RndmUniform(),Garfield::RndmUniform(), ne,ni,np);
        logger.error("Photon {}/{} has created {}e-, {}ions, {}fluorescent photons",p+1,cluster.photons.size(),ne,ni,np);
      }
      logger.warn("Now the ions !!!");
      for(std::size_t i=0;i!=cluster.ions.size();++i)
      {
        aval_MC.DriftIon(cluster.ions[i].x,cluster.ions[i].y,cluster.ions[i].z,cluster.ions[i].t);
        logger.error("Ions {}/{} at position=({:+.2e},{:+.2e},{:+.2e}) time={:+.2e} has drifted",i+1,cluster.ions.size(),cluster.ions[i].x,cluster.ions[i].y,cluster.ions[i].z,cluster.ions[i].t);
      }
      
      std::chrono::duration<double, std::milli> electron_drift_computation_time{0};
      for(const auto &electron : cluster.electrons)
      {
        static std::size_t electron_nbr{0};
        std::chrono::duration<double, std::milli> duration=timeFuncInvocation(
          [&](auto&& cluster, auto&& electron){
              logger.info("Avalanching e- {}/{} at position=({:+.2e},{:+.2e},{:+.2e}) cm, direction({:+.2e},{:+.2e},{:+.2e}) cm, time={:+.2e} ns, energy={:.2e} eV",electron_nbr+1,cluster.electrons.size(),electron.x,electron.y,electron.z,electron.dx,electron.dy,electron.dz,electron.t,electron.e);
              aval.AvalancheElectron(electron.x, electron.y, electron.z, electron.t,electron.e,electron.dx,electron.dy, electron.dz);

            }
            ,cluster,electron
        );
        logger.warn("e- {}/{} has been processed in {:+.3e}ms",electron_nbr+1,cluster.electrons.size(),duration.count());
        electron_drift_computation_time+=duration;
        if(++electron_nbr==cluster.electrons.size()) electron_nbr=0;
      }
      logger.warn("cluster {}/{}, all e- has been processed in {:+.3e}ms",cluster_nr+1,track.GetClusters().size(),electron_drift_computation_time.count());
      if(++cluster_nr==track.GetClusters().size()) cluster_nr=0;

      }
      conduction.Plot();
      conduction.GetCanvas()->Update();
      general.cd(2);
      cluster_energy.Fill(energy,1.0);
      cluster_energy.Draw();
      general.Update();
    
   // LOG("Script: Total induced charge = " << sensor.GetTotalInducedCharge("strip_0")<< " [fC].");
    //plotter_top.setName("Top side Event "+std::to_string(event));

    //plotter_bottom.setName("Bottom side Event "+std::to_string(event));

    //for(std::size_t i=0;i!=nbr_strips;++i)
    //{
    //  plotter_top.plotSignal("top_side_strip_",i);
    //  plotter_bottom.plotSignal("bottom_side_strip_",i);
    //}
  
    //plotter_top.draw();
    //plotter_bottom.draw();
  }
}
catch(...)
{

}