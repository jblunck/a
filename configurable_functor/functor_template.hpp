#ifndef FUNCTOR_HPP
#define FUNCTOR_HPP

#include <map>
#include <string>

#include "pmf_holder.hpp"

template <typename R, typename T>
class MemberFunctionFunctor
{
public:
  typedef R (T::*func_type)();

  MemberFunctionFunctor(T* obj, func_type func)
  {
    _del.bind(obj, func);
  }

  R operator()() const
  {
    return _del.get_func_ptr()( _del.get_this() );
  }

private:
  pmf_holder<T,R> _del;
};

template <typename R, typename T>
class ConfigurableFunctor :
  public T,
  public MemberFunctionFunctor<R,T>
{
public:
  ConfigurableFunctor() :
    MemberFunctionFunctor<R,T>(this, NULL)
  {
    T::set_configuration(fmap);
    set_configuration("");
  }

  void set_configuration(const std::string& conf)
  {
    typename fmap_type::const_iterator it(fmap.find(conf));

    if (it != fmap.end()) {
      MemberFunctionFunctor<R,T>::operator=
	(MemberFunctionFunctor<R, T>(this, it->second));
    }
  }

private:
  typedef std::map<std::string,
		   typename MemberFunctionFunctor<R, T>::func_type> fmap_type;
  fmap_type fmap;
};

#endif // FUNCTOR_HPP
