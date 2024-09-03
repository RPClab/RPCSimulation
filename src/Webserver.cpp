#include "RPCSim/Webserver.hpp"
#include "RPCSim/Websocket.hpp"

RPCSim::Webserver::Webserver(const std::string& options, const bool& detached)
{
  m_server.CreateEngine(options.c_str());
  if(detached) detach();
  RPCSim::Websocket handler("titi", "RPCSim websocket");
  m_server.Register("/toto", &handler);
  //tm=new TTimer(&handler, 3700);
  //tm->Start();
}

RPCSim::Webserver::~Webserver()
{
  if(!m_server.IsTerminated())
  {
    m_server.SetTerminate();
  }
}

void RPCSim::Webserver::detach()
{
  m_server.CreateServerThread();
}