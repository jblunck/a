#include "functor_virtual.hpp"

#include <iostream>
#include <string.h>

class func1;
class func2;

class Functor
{
public:
  Functor() : t(&one)
  {
  }

  int operator()(void)
  {
    return (*t)();
  }

  void set_configuration(const char *conf)
  {
    if (strcmp(conf, "two") == 0)
      t = &two;
    else
      t = &one;
  }

private:
  functor *t;

  func1 one;
  func2 two;
};


int main()
{
  Functor t;
  std::cout << t() << std::endl;
  t.set_configuration("two");
  std::cout << t() << std::endl;

  for (int i=0 ; i < 1e9; ++i)
    (void) t();

  return 0;
}
