#include "functor_bind.hpp"

#include <iostream>

#include <string.h>

struct test
{
  int do_a()
  {
    return 1;
  }

  int do_b()
  {
    return 2;
  }

  typedef int (test::*func_type)(void);

  static void set_configuration(std::map<std::string, func_type> & m)
  {
    m.insert(std::make_pair("one", &test::do_a));
    m.insert(std::make_pair("two", &test::do_b));

    // default
    m.insert(std::make_pair("", &test::do_a));
  }
};

int main()
{
  ConfigurableFunctor<test> t;
  std::cout << t() << std::endl;
  t.set_configuration("two");
  std::cout << t() << std::endl;

  for (int i=0 ; i < 1e9; ++i)
    (void) t();

  return 0;
}
