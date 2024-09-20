#ifndef RPCSIM_SETUP_H_
#define RPCSIM_SETUP_H_
#include "Garfield/ViewSignal.hh"

#include "TCanvas.h"
#include "Webserver.hpp"

namespace Garfield
{
  class Sensor;
}

namespace RPCSim
{
  class SignalPlotter
  {
  public:
    SignalPlotter()=default;
    SignalPlotter(const std::string name, const std::string title)
    {
      m_canvas.SetName(name.c_str());
      m_canvas.SetTitle(title.c_str());
    };
    void attach(Webserver& webserver);
    void setName(const std::string& name)
    { 
      m_canvas.SetName(name.c_str());
      m_canvas.SetTitle(name.c_str());
    }
    void plotSignal(const std::string& signal,const std::size_t& i);
    void setSensor(Garfield::Sensor* sensor);
    void draw()
    { 
      m_canvas.Update();
    }
  private:
    TCanvas m_canvas;
    Garfield::Sensor* m_sensor{nullptr};
    std::vector<Garfield::ViewSignal*> m_viewSignal;
  };
}

#endif