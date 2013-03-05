#include "functor_virtual.hpp"

int func1::operator()(void)
{
  __asm__("nop");
  return 1;
}

int func2::operator()(void)
{
  __asm__("nop");
  return 2;
}

