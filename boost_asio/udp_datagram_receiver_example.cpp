/*
 * echo "TEST" | socat - UDP-DATAGRAM:127.0.0.1:12345
 */

#include "udp_datagram_receiver.hpp"
#include "datagram_receiver_handler.hpp"
#include <iostream>

typedef udp_datagram_receiver<struct nop_handler,
                              struct cerr_status_listener,
                              seconds_timeout_policy> receiver_t;

template <>
bool seconds_timeout_policy<boost::asio::ip::udp>::on_timeout(
    boost::asio::basic_datagram_socket<boost::asio::ip::udp>& socket) const
{
    std::cout << "timeout: " << socket.local_endpoint().address().to_string()
              << ":" << socket.local_endpoint().port()
              << std::endl;
    return true;
}

int main()
{
    struct nop_handler nh;
    struct cerr_status_listener mc;
    boost::shared_ptr<receiver_t> receiver(new receiver_t(nh, mc));
    receiver->set_timer_interval(5);
    receiver->bind(12345);
    receiver->run();
    return 0;
}
