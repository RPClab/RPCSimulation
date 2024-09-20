#pragma once

#include <string>
#include <iostream>
#include <thread>
#include "THttpServer.h"

namespace RPCSim
{
  class WebServer
  {
    public:
     WebServer()=default;
     WebServer(const std::string& options, const bool& detach=false);
     virtual ~WebServer()
     {
      if(m_server)
      {
        m_server->SetTerminate();
        while(!m_server->IsTerminated()) std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(100));
        delete m_server;
      } 
     }
     void bind(const std::string& path,const bool& detach=false);
     void detach() { if(m_server)m_server->CreateServerThread();}
     bool attach(const std::string& subfolder,const TObject& obj);
     THttpServer& operator()() {return *m_server;}
    private:
    THttpServer* m_server{nullptr};
  };
}