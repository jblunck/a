#ifndef __DATAGRAM_RECEIVER_HPP__
#define __DATAGRAM_RECEIVER_HPP__

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

/*
struct EventHandler
{
  typedef void data_type;
  static size_t data_type_size();

  void handle_receive(data_type buffer, std::size_t length);
  data_type get_next(data_type prev);
};

struct StatusListener
{
  void error(const std::string & msg);
};
*/

template <class EventHandler,
	  class StatusListener,
	  typename Protocol>
class datagram_receiver :
  public boost::enable_shared_from_this<datagram_receiver<EventHandler,StatusListener,Protocol> >
{
    typedef typename EventHandler::data_type data_type;
    EventHandler& _handler;

    StatusListener& _status;

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
	data_type next = _handler.get_next(buffer);
	socket.async_receive(boost::asio::buffer(next,
						 EventHandler::data_type_size()),
			     boost::bind(&datagram_receiver::handle_async_receive,
					 boost::enable_shared_from_this<datagram_receiver<EventHandler,StatusListener,Protocol> >::shared_from_this(),
					 boost::asio::placeholders::error,
					 boost::ref(socket),
					 boost::ref(next),
					 boost::asio::placeholders::bytes_transferred));
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

	_handler(buffer, length);
	start_receive(socket, buffer);
    }

};

#endif // __DATAGRAM_RECEIVER_HPP__
