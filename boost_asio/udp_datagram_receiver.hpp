#ifndef __UDP_DATAGRAM_RECEIVER_HPP__
#define __UDP_DATAGRAM_RECEIVER_HPP__
#include "datagram_receiver.hpp"

/*
 * Partitioning the network processing is important:
 * - Different sub-feeds (I/S) must be processed on the same interface. They
 *   usually originate from the same front-end processor.
 * - Different feeds (A/B) should be processed on different interfaces since
 *   they usually originate from different front-end processing systems. If
 *   they arrive on the same interface, only one feed should be processed at a
 *   time.
 * - Different products (channel 7/9) could be processed on different interfaces
 *   if it is known that they orginate from different front-end processors.
 * - Only one thread should service one physical interface at a time. (?)
 */
/*
 * udp_multicast_receiver:
 * - run(): thread of handler execution
 * - join(),leave(): multicast group concept, one socket per port
 */
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

// turn to use template basic_datagram_socket<Protocol>

template <class EventHandler,
	  class StatusListener>
class udp_datagram_receiver : public datagram_receiver<EventHandler,
						       StatusListener,
						       boost::asio::ip::udp>
{
    typedef datagram_receiver<EventHandler,StatusListener,boost::asio::ip::udp> base_type;
    boost::asio::ip::udp::socket _socket;
    const boost::asio::ip::address _interface_address;
    const bool _is_loopback;

public:
    udp_datagram_receiver(EventHandler& handler,
			  StatusListener& listener) :
        datagram_receiver<EventHandler,StatusListener,boost::asio::ip::udp>(handler, listener),
	_socket(base_type::get_io_service()),
	_interface_address(),
	_is_loopback(false)
    {
    }

    udp_datagram_receiver(EventHandler& handler,
			  StatusListener& listener,
			  const char * interface_address,
			  const bool is_loopback) :
        datagram_receiver<EventHandler,StatusListener,boost::asio::ip::udp>(handler, listener),
	_socket(base_type::get_io_service()),
        _interface_address(boost::asio::ip::address_v4::from_string(interface_address)),
	_is_loopback(is_loopback)
    {
    }

    /*
     * \exception std::runtime_error
     */
    void bind(const unsigned short port)
    {
        boost::asio::ip::udp::endpoint ep(boost::asio::ip::udp::v4(), port);

	_socket.open(ep.protocol());

	boost::system::error_code ec;
	boost::asio::ip::udp::socket::reuse_address opt_reuse(true);
	_socket.set_option(opt_reuse, ec);
	if (ec)
	    throw std::runtime_error(ec.message());

	_socket.bind(ep);

	// start receiving function
	base_type::start_receive(_socket, NULL);
    }

};

#endif // __UDP_DATAGRAM_RECEIVER_HPP__
