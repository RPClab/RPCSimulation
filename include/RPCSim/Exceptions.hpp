#ifndef RPCSIM_EXCEPTION_H_
#define RPCSIM_EXCEPTION_H_
#include <exception>
#include <string>
namespace RPCSim
{

class exception : public std::exception
{
public:
  exception() noexcept =default;
  exception(const std::string& what) : std::exception()
  {
    m_what.resize(what.size());
    m_what=what;
  }
  virtual const char * what() const noexcept override
  {
    if(m_what.empty()) return "RPCSim::exception";
    else return m_what.c_str();
  }
  virtual ~exception() noexcept =default;
  std::string m_what;
};

}

#endif