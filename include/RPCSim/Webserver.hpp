#ifndef RPCSIM_WEBSERVER_H_
#define RPCSIM_WEBSERVER_H_

#include <string>
#include "THttpServer.h"

namespace RPCSim 
{
  class Webserver
  {
public:
  Webserver()=default;
  Webserver(const std::string& options, const bool& detach=false);
  Webserver(const Webserver&) =delete;
  Webserver& operator=(const Webserver& other) =delete;
  virtual ~Webserver();
  void detach();
  THttpServer& operator()() noexcept{ return m_server; }
private:
    THttpServer m_server;
    TTimer* tm;
  };
}

#endif