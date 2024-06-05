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

using namespace Garfield;

int main(int argc, char *argv[]) try
{
  // Setup all this mess
  RPCSim::setEnv();
  TApplication app("app", &argc, argv);
  //plottingEngine.SetDefaultStyle();

  RPCSim::RPCGeometry my_rpc;

  // Setup the gas, but one can also use a gasfile.
  RPCSim::GasMixture gasMixture(RPCSim::data_folder+"/examples/c2h2f4_94-7_iso_5_sf6_0-3_bis.gas");
  //gasMixture.generate(7000.0,0.,0.);


  my_rpc.fillGasGap(&gasMixture);
  my_rpc.build();
  my_rpc.draw();
  

Garfield::ComponentAnalyticField cmp;
cmp.SetMedium(gasMixture.getMagboltzMedium());
cmp.AddPlaneY(0,0,"Anode");
cmp.AddPlaneY(0.32,-6600,"Cathode");
constexpr double pitch = 4;
constexpr double extar=0.1;
const std::size_t nbr_strips{12};
for(std::size_t i=0;i!=nbr_strips;++i)
{
  cmp.AddStripOnPlaneY('z', 0.32, i*pitch+(i*extar), (i+1)*pitch+(i*extar), "strip_"+std::to_string(i));
  std::cout<<"Position start : "<<i*pitch+(i*extar)<<" "<<" end : "<<(i+1)*pitch+(i*extar)<<std::endl;
}

  Garfield::Sensor sensor;
  // Calculate the electric field using the Component object cmp.
  sensor.AddComponent(&cmp);
  // Request signal calculation for the electrode named "s",
  // using the weighting field provided by the Component object cmp.
  for(std::size_t i=0;i!=12;++i)
  {
    sensor.AddElectrode(&cmp, "strip_"+std::to_string(i));
  }
  const unsigned int nTimeBins = 30000;
  const double tmin = 0.;
  const double tmax = 4;
  const double tstep = 0.001;//(tmax - tmin) / nTimeBins;
  sensor.SetTimeWindow(tmin, tstep, nTimeBins);

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
