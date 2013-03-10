#include "timeout_policy.hpp"
#include <boost/asio.hpp>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

namespace
{

struct fixture
{
    fixture() : socket(io_service)
    {
    }

    typedef boost::asio::local::datagram_protocol protocol_type;
    boost::asio::io_service io_service;
    boost::asio::local::datagram_protocol::socket socket;
};

BOOST_FIXTURE_TEST_CASE(never_timeout_policy, fixture)
{
    ::never_timeout_policy<protocol_type> ntp;
    ::never_timeout_policy<protocol_type>::functor_type f(ntp, socket);

    BOOST_CHECK_NO_THROW(f());
}

template <typename T>
struct test_policy
{
    test_policy() : interval(0), restart(true), called(0)
    {
    }

    int get_timer_interval() const
    {
      return interval;
    }

    bool on_timeout(boost::asio::basic_datagram_socket<T>& socket) const
    {
        ++called;
        return restart;
    }

    int interval;
    bool restart;
    mutable unsigned int called;
};

BOOST_FIXTURE_TEST_CASE(timer_functor_empty, fixture)
{
    test_policy<protocol_type> t;
    ::timer_functor<test_policy<protocol_type>, protocol_type> f(t, socket);

    BOOST_CHECK_NO_THROW(io_service.run_one());
    BOOST_CHECK_EQUAL(t.called, 1);

    // call the functor so that we do not timeout
    BOOST_CHECK_NO_THROW(f());
    BOOST_CHECK_NO_THROW(io_service.run_one());
    BOOST_CHECK_EQUAL(t.called, 1);

    t.restart = false;
    BOOST_CHECK_NO_THROW(io_service.run_one());
    BOOST_CHECK_EQUAL(t.called, 2);
    BOOST_CHECK_NO_THROW(io_service.run_one());
    BOOST_CHECK_EQUAL(t.called, 2);
}

} // namespace anonymous
