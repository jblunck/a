#ifndef __DATAGRAM_RECEIVER_HPP__
#define __DATAGRAM_RECEIVER_HPP__

#include "buffer_policy.hpp"
#include "timeout_policy.hpp"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/utility.hpp>

/*
 * Object Lifetime of the datagram_receiver<>
 *
 * The stream of execution starts when a new socket is created by the join()
 * method of the datagram_receiver<>. The initialization function is the
 * start_receiver() method of the datagram_receiver<>.
 *
 * The asynchronous handler is a method of the datagram_receiver<> as well and
 * therefore needs to life longer than the socket and the io_service. The
 * asynchronous handler is dispatching the real work to the EventHandler
 * object that exists once per datagram_receiver<>. In case of an error the
 * StatusListener object is called that exists once per datagram_receiver<>.
 *
 * struct EventHandler
 * {
 *   void operator()(char *buffer, std::size_t length);
 * };
 *
 * struct StatusListener
 * {
 *   void error(const std::string & msg);
 * };
 */

template <class EventHandler,
	  class StatusListener,
	  typename Protocol,
          template <class> class TimeoutPolicy = never_timeout_policy,
          typename BufferPolicy = static_buffer_policy<64 * 1024> >
class datagram_receiver : boost::noncopyable,
  public boost::enable_shared_from_this<datagram_receiver<EventHandler,
                                                          StatusListener,
                                                          Protocol,
                                                          TimeoutPolicy,
                                                          BufferPolicy> >
{
    typedef boost::enable_shared_from_this<datagram_receiver<EventHandler,
                                                             StatusListener,
                                                             Protocol,
                                                             TimeoutPolicy,
                                                             BufferPolicy> > super_type;

    EventHandler& _handler;

    StatusListener& _status;

    typedef typename BufferPolicy::data_type data_type;
    BufferPolicy _policy;

    boost::asio::io_service _io_service;

public:
    datagram_receiver(EventHandler& handler,
		      StatusListener& listener)
	: _handler(handler),
	  _status(listener)
    {
    }

    boost::asio::io_service & get_io_service()
    {
        return _io_service;
    }

    void run()
    {
	_io_service.run();
    }

    struct datagram_receiver_session
    {
        typedef boost::asio::basic_datagram_socket<Protocol> socket_type;
        boost::reference_wrapper<socket_type> _socket;
        boost::shared_ptr<TimeoutPolicy<Protocol> > _timeout;

        datagram_receiver_session(socket_type & socket)
            : _socket(socket),
              _timeout(new TimeoutPolicy<Protocol>(socket))
        {
        }

        socket_type & get_socket()
        {
            return _socket;
        }

        void keep_alive()
        {
            (*_timeout)();
        }
    };

    typedef struct datagram_receiver_session session_type;

    void start_receive(session_type session, data_type buffer)
    {
        typename session_type::socket_type& s = session.get_socket();
	s.async_receive(buffer,
                        boost::bind(&datagram_receiver::handle_async_receive,
                                    super_type::shared_from_this(),
                                    boost::asio::placeholders::error,
                                    session,
                                    buffer,
                                    boost::asio::placeholders::bytes_transferred));
    }

    void start_receive(boost::asio::basic_datagram_socket<Protocol>& socket)
    {
	start_receive(session_type(socket), _policy());
    }

    void stop_receive()
    {
    }

private:
    /**
     * IO completion callback
     */
    void handle_async_receive(const boost::system::error_code& error,
			      session_type session,
			      data_type buffer,
			      size_t length)
    {
	if (error) {
            if (error != boost::asio::error::operation_aborted)
                _status.error(error.message());
	    return;
	}

	// trigger session on every receive
        session.keep_alive();

	_handler(boost::asio::buffer_cast<char *>(*buffer.begin()), length);

	start_receive(session, buffer);
    }

};

#endif // __DATAGRAM_RECEIVER_HPP__
