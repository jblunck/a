#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <map>
#include <string>

template <typename T>
class ConfigurableFunctor :
  public T
{
public:
  int operator()(void)
  {
    __asm__("nop");
    return _func();
  }

  ConfigurableFunctor()
  {
    T::set_configuration(fmap);
    set_configuration("");
  }

  void set_configuration(const char *conf)
  {
    typename fmap_type::const_iterator it(fmap.find(conf));

    if (it != fmap.end()) {
      _func = boost::bind(it->second, static_cast<T*>(this));
    }
  }

private:
  typedef std::map<std::string, typename T::func_type> fmap_type;
  fmap_type fmap;

  boost::function<int(void)> _func;
};
