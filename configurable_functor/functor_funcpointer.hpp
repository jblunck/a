#ifndef FUNCTOR_HPP
#define FUNCTOR_HPP

#include <map>
#include <string>

template <typename T>
class ConfigurableFunctor
{
public:
  int operator()(void)
  {
    __asm__ volatile("nop");
    int r( (_t.*_func)() );
    __asm__ volatile("nop");
    return r;
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
      _func = it->second;
    }
  }

private:
  typedef std::map<std::string,typename T::func_type> fmap_type;
  fmap_type fmap;

  T _t;
  int (T::*_func)(void);
};

#endif
