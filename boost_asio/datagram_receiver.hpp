#ifndef __DATAGRAM_RECEIVER_HPP__
#define __DATAGRAM_RECEIVER_HPP__

#include "buffer_policy.hpp"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

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
          typename BufferPolicy = static_buffer_policy<64 * 1024> >
class datagram_receiver :
  public boost::enable_shared_from_this<datagram_receiver<EventHandler,
                                                          StatusListener,
                                                          Protocol,
                                                          BufferPolicy> >
{
    EventHandler& _handler;

    StatusListener& _status;

    typedef typename BufferPolicy::data_type data_type;
    BufferPolicy _policy;

    boost::asio::io_service _io_service;

public:
    datagram_receiver(EventHandler& handler, StatusListener& listener) :
	_handler(handler),
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

    void start_receive(boost::asio::basic_datagram_socket<Protocol>& socket,
		       data_type buffer)
    {
	socket.async_receive(buffer,
			     boost::bind(&datagram_receiver::handle_async_receive,
					 boost::enable_shared_from_this<datagram_receiver<EventHandler,StatusListener,Protocol,BufferPolicy> >::shared_from_this(),
					 boost::asio::placeholders::error,
					 boost::ref(socket),
					 buffer,
					 boost::asio::placeholders::bytes_transferred));
    }

    void start_receive(boost::asio::basic_datagram_socket<Protocol>& socket)
    {
        start_receive(socket, _policy());
    }

    void stop_receive()
    {
    }

private:
    /**
     * IO completion callback
     */
    void handle_async_receive(const boost::system::error_code& error,
			      boost::asio::basic_datagram_socket<Protocol>& socket,
			      data_type buffer,
			      size_t length)
    {
	if (error) {
	    _status.error(error.message());
	    return;
	}

	_handler(boost::asio::buffer_cast<char *>(*buffer.begin()),
                 length);
	start_receive(socket, buffer);
    }

};

#endif // __DATAGRAM_RECEIVER_HPP__
