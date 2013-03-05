#ifndef PMF_HOLDER_HPP
#define PMF_HOLDER_HPP

/*
 * Thanks to matz for enlightenment about pointer to member functions with G++
 *
 * This class is purely for optimization of the pointer to member function
 * call. I don't want to pay the price of calculating the "real" function
 * pointer address each time the MemberFunctionFunctor::operator() is called.
 * Therefore we do the calculation of the "real" address to call when binding.
 */

#include <cassert>
#include <cstddef>

#if 0
#include <cstdio>
#define DEBUG(...) std::printf(__VA_ARGS__)
#else
#define DEBUG(...) do { } while(0)
#endif

template <typename T, typename R>
class pmf_holder
{
  // this is how a pointer to a member function really looks like (GCC)
  struct pmf_t {
    void* ptr_or_slot;
    ptrdiff_t this_adjustment;
  };

  // helper union for access to pointer to member function details
  union {
    R (T::*ptr)();
    struct pmf_t pmf;
  } u;

  void* thisptr;
  void* funcptr;

public:
  pmf_holder() :
    thisptr(NULL),
    funcptr(NULL)
  {
  }

  pmf_holder(T *obj, R (T::*func)())
  {
    // TODO: move this to compile time
    assert(sizeof(func) == sizeof(struct pmf_t));

    bind(obj, func);
  }

  T * get_this() const
  {
    return (T*)thisptr;
  }

  typedef R (*func_ptr_t)(T*);
  func_ptr_t get_func_ptr() const
  {
    return (func_ptr_t)funcptr;
  }

  void bind(T *obj, R (T::*func)())
  {
    u.ptr = func;

    funcptr = u.pmf.ptr_or_slot;
    thisptr = obj;
    thisptr = reinterpret_cast<char *>(thisptr) + u.pmf.this_adjustment;

    if (((unsigned long)u.pmf.ptr_or_slot) & 0x1) {
      void *vtable = (void*)*reinterpret_cast<size_t *>(thisptr);
      DEBUG("vtable: %p\n", vtable);

      // offset in vtable
      size_t offset = reinterpret_cast<size_t>(funcptr) - 1;
      DEBUG("offset: %d\n", offset);

      void *vtableptr = (void*)(reinterpret_cast<char *>(vtable) + offset);
      DEBUG("vtableptr: %p\n", vtableptr);

      funcptr = (void*)*reinterpret_cast<size_t *>(vtableptr);
      DEBUG("funcptr: %p\n", funcptr);
    }
  }

};

#undef DEBUG

#endif
