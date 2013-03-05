#include "functor_template.hpp"

#include <iostream>

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

  virtual int do_c()
  {
    return 0;
  }

  typedef int (test::*func_type)(void);

  static void set_configuration(std::map<std::string, func_type> & m)
  {
    m.insert(std::make_pair("one", &test::do_a));
    m.insert(std::make_pair("two", &test::do_b));
    m.insert(std::make_pair("three", &test::do_c));

    // default
    m.insert(std::make_pair("", &test::do_a));
  }
};

class v : public test
{
public:
  virtual int do_c()
  {
    return 3;
  }
};

int main()
{
  ConfigurableFunctor<int, test> t;
  std::cout << t() << std::endl;
  t.set_configuration("two");
  std::cout << t() << std::endl;

  for (int i=0 ; i < 1e9; ++i)
    (void) t();

  return 0;
}
