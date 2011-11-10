#ifndef __TIMEOUT_POLICY_HPP__
#define __TIMEOUT_POLICY_HPP__

#include <boost/asio.hpp>
#include <boost/bind.hpp>

template <typename T>
struct never_timeout_policy
{
    never_timeout_policy(boost::asio::basic_datagram_socket<T>& socket)
    {
    }

    // called on receive event
    void operator()()
    {
    }
};

template <typename T, std::size_t N>
struct always_timeout_policy
{
    boost::asio::deadline_timer timer;

    always_timeout_policy(boost::asio::basic_datagram_socket<T>& socket)
        : timer(socket.get_io_service(), boost::posix_time::seconds(N))
    {
        timer.async_wait(boost::bind(&always_timeout_policy::on_timeout,
                                     boost::ref(*this),
                                     boost::asio::placeholders::error));
    }

    // called on receive event
    void operator()()
    {
    }

    void on_timeout(const boost::system::error_code& error)
        {
            if (error == boost::asio::error::operation_aborted)
                return;

            std::cerr << "TIMEOUT" << std::endl;

            // Re-arm timer
            timer.expires_from_now(boost::posix_time::seconds(N));
            timer.async_wait(boost::bind(&always_timeout_policy::on_timeout,
                                         boost::ref(*this),
                                         boost::asio::placeholders::error));
        }
};

#if 0
template <int N, typename CheckPolicy, typename Handler>
struct seconds_timeout_policy
{
    boost::asio::deadline_timer timer;
    CheckPolicy _policy;
    Handler _handler;
    seconds_timeout_policy(Handler handler)
        : timer(get_io_service()),
          _handler(handler)
    {
        timer.expires_from_now(boost::posix_time::seconds(N));
        timer.async_wait(handler);
    }

    void handler(const boost::system::error_code& error)
    {
        if (error == boost::asio::error::operation_aborted)
            return;

        // Timer was not cancelled, take necessary action.
        if (policy())
            _handler();

        // Re-arm timer
        timer.expires_from_now(boost::posix_time::seconds(N));
        timer.async_wait(handler);
    }
};
#endif

#endif // __TIMEOUT_POLICY_HPP__
