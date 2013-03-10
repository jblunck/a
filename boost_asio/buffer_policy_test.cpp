#include "buffer_policy.hpp"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using boost::asio::detail::buffer_cast_helper;

BOOST_AUTO_TEST_CASE(static_buffers)
{
  static_buffer_policy<1024> p;
  static_buffer_policy<1024>::data_type t1(p());
  static_buffer_policy<1024>::data_type t2(p());

  void *p1 = buffer_cast_helper(*t1.begin());
  void *p2 = buffer_cast_helper(*t2.begin());
  BOOST_CHECK(p1 == p2);

  static_buffer_policy<1024>::data_type copy(t2);
  void *c1 = buffer_cast_helper(*copy.begin());
  BOOST_CHECK(c1 == p2);  
}

BOOST_AUTO_TEST_CASE(shared_buffers)
{
  shared_buffer_policy<1024> p;
  shared_buffer_policy<1024>::data_type t1(p());
  shared_buffer_policy<1024>::data_type t2(p());

  void *p1 = buffer_cast_helper(*t1.begin());
  void *p2 = buffer_cast_helper(*t2.begin());
  BOOST_CHECK(p1 != p2);
}

BOOST_AUTO_TEST_CASE(shared_buffers_equal)
{
  shared_buffer_policy<1024> p;
  shared_buffer_policy<1024>::data_type t1(p());
  shared_buffer_policy<1024>::data_type t2(t1);

  void *p1 = buffer_cast_helper(*t1.begin());
  void *p2 = buffer_cast_helper(*t2.begin());
  BOOST_CHECK(p1 == p2);
}
