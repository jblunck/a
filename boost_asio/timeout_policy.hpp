#ifndef __TIMEOUT_POLICY_HPP__
#define __TIMEOUT_POLICY_HPP__

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

template <typename P, typename T>
struct empty_functor {
    empty_functor(const P&, boost::asio::basic_datagram_socket<T>&)
    {
    }

    // called on receive event
    void operator()()
    {
    }
};

template <typename T>
struct never_timeout_policy
{
    never_timeout_policy()
    {
    }

    typedef struct empty_functor<never_timeout_policy<T>, T> functor_type;
};

template <typename P, typename T>
struct timer_functor
{
    boost::posix_time::seconds _timer_interval;
    boost::asio::deadline_timer _timer;
    bool _functor_called;
    boost::function<bool(void)> _timeout_handler;

    timer_functor(const P& policy,
                  boost::asio::basic_datagram_socket<T>& socket)
        : _timer_interval(policy.get_timer_interval())
        ,_timer(socket.get_io_service(), _timer_interval)
        , _functor_called(false)
    {
        _timeout_handler = boost::bind(&P::on_timeout,
                                       boost::cref(policy),
                                       boost::ref(socket));

        _timer.async_wait(boost::bind(&timer_functor::on_timeout,
                                      boost::ref(*this),
                                      boost::asio::placeholders::error));
    }

    // called on timer expire
    void on_timeout(const boost::system::error_code& error)
    {
        if (error == boost::asio::error::operation_aborted)
            return;

        if (!_functor_called && !_timeout_handler())
            return;

        // Re-arm timer
        _functor_called = false;
        _timer.expires_from_now(_timer_interval);
        _timer.async_wait(boost::bind(&timer_functor::on_timeout,
                                      boost::ref(*this),
                                      boost::asio::placeholders::error));
    }

    // called on receive event
    void operator()()
    {
        _functor_called = true;
    }
};

template <typename T>
class seconds_timeout_policy
{
public:
    typedef struct timer_functor<seconds_timeout_policy<T>, T> functor_type;

    seconds_timeout_policy() :
        _interval(30)
    {
    }

    void set_timer_interval(unsigned int interval)
    {
        _interval = interval;
    }

    unsigned int get_timer_interval() const
    {
        return _interval;
    }

    // the definition needs to be provided by the user
    bool on_timeout(boost::asio::basic_datagram_socket<T>& socket) const;

private:
    unsigned int _interval;
};

#endif // __TIMEOUT_POLICY_HPP__
