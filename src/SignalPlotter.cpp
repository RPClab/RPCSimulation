#include "RPCSim/SignalPlotter.hpp"
#include <Garfield/Sensor.hh>
#include <TVirtualPad.h>

void RPCSim::SignalPlotter::setSensor(Garfield::Sensor* sensor)
{
  m_sensor=sensor;
  m_canvas.Divide(10,2);
}

void RPCSim::SignalPlotter::attach(Webserver& webserver)
{
  webserver().Register("/", &m_canvas);
  m_canvas.Draw();
  m_canvas.Update();
}

void RPCSim::SignalPlotter::plotSignal(const std::string& signal,const std::size_t& i)
{
  m_viewSignal.push_back(new Garfield::ViewSignal);
  m_viewSignal[m_viewSignal.size()-1]->SetSensor(m_sensor);
  m_canvas.cd((int)(i+1));
  m_viewSignal[m_viewSignal.size()-1]->SetCanvas(reinterpret_cast<TPad*>(gPad));
  m_viewSignal[m_viewSignal.size()-1]->PlotSignal(signal+std::to_string(i),"tei","tei","tei");
}