#ifndef __DATAGRAM_RECEIVER_HPP__
#define __DATAGRAM_RECEIVER_HPP__

#include "buffer_policy.hpp"

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
          typename BufferPolicy = static_buffer_policy<64 * 1024> >
class datagram_receiver : boost::noncopyable
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

  /*
   * Object of the session class need to be cheap to copy because they are
   * copied into the asynchronous callback of the ASIO library.
   */
  class datagram_receiver_session:
    public boost::enable_shared_from_this<datagram_receiver_session>
  {
    typedef boost::enable_shared_from_this<datagram_receiver_session> super_type;

    typedef boost::asio::basic_datagram_socket<Protocol> socket_type;
    socket_type& _socket;

    // our own copy of this buffer instance
    data_type _buffer;

    EventHandler& _handler;
    StatusListener& _status;

  public:
    datagram_receiver_session(socket_type & socket,
			      data_type & buffer,
			      EventHandler& handler,
			      StatusListener& status)
      : _socket(socket),
	_buffer(buffer),
	_handler(handler),
	_status(status)
    {
    }

    socket_type & get_socket()
    {
      return _socket;
    }

  private:
    data_type & get_buffer()
    {
      return _buffer;
    }

    /**
     * IO completion callback expected by ASIO
     */
    void handle_async_receive(const boost::system::error_code& error_code,
                              std::size_t bytes_transfered)
    {
      // one central status listener per datagram_receiver
      if (error_code) {
          if (error_code != boost::asio::error::operation_aborted)
              _status.error(error_code.message());
          return;
      }

      data_type& buffer = get_buffer();
      _handler(boost::asio::buffer_cast<char *>(*buffer.begin()),
               bytes_transfered);

      start_receive(buffer);
    }

  public:
    /*
     * Function to start the next asynchronous receive operation
     */
    void start_receive(data_type buffer)
    {
      socket_type& s = get_socket();

      // register this as read handler function object
      s.async_receive(buffer,
		      boost::bind(&datagram_receiver_session::handle_async_receive,
				  datagram_receiver_session::shared_from_this(),
				  boost::asio::placeholders::error,
				  boost::asio::placeholders::bytes_transferred)
		      );
    }
  };

  typedef boost::shared_ptr<datagram_receiver_session> session_type;

  void start_receive(boost::asio::basic_datagram_socket<Protocol>& socket)
  {
    data_type buffer(_policy());

    session_type s(new datagram_receiver_session(socket, buffer,
                                                 _handler, _status));
    s->start_receive(buffer);
  }

  void stop_receive()
  {
  }
};

#endif // __DATAGRAM_RECEIVER_HPP__
