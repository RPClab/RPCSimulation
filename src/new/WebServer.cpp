#include "RPCSim/new/WebServer.hpp"

RPCSim::WebServer::WebServer(const std::string& options, const bool& detached)
{
  bind(options,detached);
}


void RPCSim::WebServer::bind(const std::string& path,const bool& detached)
{
  if(!m_server)
  {
    m_server=new THttpServer(path.c_str());
    if(detached) detach();
  }
}

bool RPCSim::WebServer::attach(const std::string& subfolder,const TObject& obj)
{
  if(m_server)
  {
    return m_server->Register(subfolder.c_str(),const_cast<TObject*>(&obj));
  }
  else return false;
}