#ifndef RPCSIM_MEDIUM_H_
#define RPCSIM_MEDIUM_H_
class TGeoElementTable;
class TGeoMedium;

namespace RPCSim
{
class Medium
{
public:
  Medium();
  virtual ~Medium() = default;
  TGeoMedium* operator()()
  {
    return m_medium;
  }
protected:
  TGeoMedium* m_medium{nullptr};
  static TGeoElementTable* m_elementTable;
private:
  void getElementTable();
};


class Glass final : public Medium
{
public:
  Glass();
};

class Graphite final : public Medium
{
  public:
  Graphite();
};

class Mylar final : public Medium
{
  public:
  Mylar();
};

}
#endif