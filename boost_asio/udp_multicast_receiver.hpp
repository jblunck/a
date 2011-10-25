#ifndef __UDP_MULTICAST_RECEIVER_HPP__
#define __UDP_MULTICAST_RECEIVER_HPP__

#include "datagram_receiver.hpp"
#include <map>

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

template <class EventHandler,
	  class StatusListener>
class udp_multicast_receiver : public datagram_receiver<EventHandler,
						       StatusListener,
						       boost::asio::ip::udp>
{
    typedef std::map<boost::asio::ip::udp::endpoint,
		     boost::shared_ptr<boost::asio::ip::udp::socket> >::iterator socket_map_iterator;
    std::map<boost::asio::ip::udp::endpoint,
	     boost::shared_ptr<boost::asio::ip::udp::socket> > _socket_map;

    const boost::asio::ip::address _interface_address;
    const bool _is_loopback;

public:
    udp_multicast_receiver(EventHandler& handler,
			   StatusListener& listener) :
        datagram_receiver<EventHandler,StatusListener,boost::asio::ip::udp>(handler, listener),
	_interface_address(),
	_is_loopback(false)
    {
    }

    udp_multicast_receiver(EventHandler& handler,
			   StatusListener& listener,
			   const char * interface_address,
			   const bool is_loopback) :
        datagram_receiver<EventHandler,StatusListener,boost::asio::ip::udp>(handler, listener),
        _interface_address(boost::asio::ip::address_v4::from_string(interface_address)),
	_is_loopback(is_loopback)
    {
    }

    /*
     * \exception std::runtime_error
     */
    void join(const char * address, const unsigned short port)
    {
	boost::shared_ptr<boost::asio::ip::udp::socket> socket;

	// FIXME: We should not new here but reserve this on startup!
	socket.reset(new boost::asio::ip::udp::socket(datagram_receiver<EventHandler,
						       StatusListener,
									boost::asio::ip::udp>::get_io_service()));

	boost::asio::ip::udp::endpoint ep(boost::asio::ip::address_v4::from_string(address), port);

	// first check if we have already joined this endpoint
	socket_map_iterator it = _socket_map.find(ep);
	if (it != _socket_map.end())
	    throw std::runtime_error("Already joined");

	socket->open(ep.protocol());

	/*
	 * Before we join the multicast group we need to prepare the socket:
	 *
	 * 1.) Never exclusively claim the address.
	 */
	boost::system::error_code ec;
	boost::asio::ip::udp::socket::reuse_address opt_reuse(true);
	socket->set_option(opt_reuse, ec);
	if (ec)
	    throw std::runtime_error(ec.message());

	/*
	 * 2.) If we want to receive packet that have been sent from the same
	 * host that we are on, we need to enable additional options first.
	 */
	if (_is_loopback) {
	    boost::asio::ip::multicast::enable_loopback opt_loopback(true);
	    socket->set_option(opt_loopback, ec);
	    if (ec)
		throw std::runtime_error(ec.message());
	}

	socket->bind(ep);

	// join the multicast group
	boost::asio::ip::multicast::join_group opt_join(ep.address().to_v4(),
							_interface_address.to_v4());
	socket->set_option(opt_join, ec);
	if (ec)
	    throw std::runtime_error(ec.message());

	// keep socket so that we can leave properly later
	_socket_map[ep] = socket;

	// start receiving function
	datagram_receiver<EventHandler,
			  StatusListener,
			  boost::asio::ip::udp>::start_receive(*socket);
    }

    /*
     * \exception std::runtime_error
     */
    void leave(const char * address, const unsigned short port)
    {
	boost::asio::ip::udp::endpoint ep(boost::asio::ip::address_v4::from_string(address), port);

	// first check if we have already joined this endpoint
	socket_map_iterator it = _socket_map.find(ep);
	if (it == _socket_map.end())
	    throw std::runtime_error("Not joined");

	boost::shared_ptr<boost::asio::ip::udp::socket> socket = it->second;
	_socket_map.erase(it);

	boost::system::error_code ec;
	socket->close(ec);
	if (ec)
	    throw std::runtime_error(ec.message());
    }

    /*
     * recv.join_and_process(s_channel.address(), s_channel.port(),
     *                       queueing_snapshot_handler);
     */
    void join_and_process(unsigned short port, EventHandler& handler)
    {
	// create socket
	// join
	// start_receive(handler)
    }

};

#endif // __UDP_MULTICAST_RECEIVER_HPP__
