#include <boost/asio.hpp>
#include <boost/bind.hpp>
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
struct EventHandler
{
  typedef void data_type;
  static size_t data_type_size();

  void handle_receive(data_type buffer, std::size_t length);
  data_type get_next(data_type prev);
};

struct ConnectionHandler
{
  static void error(const std::string & msg);
};
*/

template <class EventHandler,
	  class ConnectionHandler>
class udp_multicast_receiver
{
    typedef typename EventHandler::data_type data_type;
    EventHandler& _handler;

    boost::asio::io_service _io_service;

    typedef std::map<boost::asio::ip::udp::endpoint,
		     boost::shared_ptr<boost::asio::ip::udp::socket> >::iterator socket_map_iterator;
    std::map<boost::asio::ip::udp::endpoint,
	     boost::shared_ptr<boost::asio::ip::udp::socket> > _socket_map;

    const boost::asio::ip::address _interface_address;
    const bool _is_loopback;
public:
    udp_multicast_receiver(EventHandler& handler) :
	_handler(handler),
	_interface_address(),
	_is_loopback(false)
    {
    }

    udp_multicast_receiver(EventHandler& handler,
			   const char * interface_address,
			   const bool is_loopback) :
	_handler(handler),
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
	socket.reset(new boost::asio::ip::udp::socket(_io_service));

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
	start_receive(*socket, NULL);
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

    void join_socket(int socket)
    {
	//_socket.assign(boost::asio::ip::udp::v4(), socket);
	// start receiving function
	//start_receive(NULL);
    }

    void run()
    {
	_io_service.run();
    }

private:
    /**
     * IO completion callback
     */
    void handle_async_receive(const boost::system::error_code& error,
			      boost::asio::ip::udp::socket& socket,
			      data_type buffer,
			      size_t length)
    {
	if (error) {
	    ConnectionHandler::error(error.message());
	    return;
	}

	_handler(buffer, length);
	start_receive(socket, buffer);
    }

    void start_receive(boost::asio::ip::udp::socket & socket, data_type buffer)
    {
	data_type next = _handler.get_next(buffer);
	socket.async_receive(boost::asio::buffer(next,
						 EventHandler::data_type_size()),
			     boost::bind(&udp_multicast_receiver::handle_async_receive,
					 this,
					 boost::asio::placeholders::error,
					 boost::ref(socket),
					 boost::ref(next),
					 boost::asio::placeholders::bytes_transferred));
    }

    void stop_receive()
    {
    }

};
