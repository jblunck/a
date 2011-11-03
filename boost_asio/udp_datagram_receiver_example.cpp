/*
 * echo "TEST" | socat - UDP-DATAGRAM:127.0.0.1:12345
 */

#include "udp_datagram_receiver.hpp"
#include "datagram_receiver_handler.hpp"
#include <iostream>

typedef udp_datagram_receiver<struct nop_handler,
                              struct cerr_status_listener> receiver_t;

int main()
{
    struct nop_handler nh;
    struct cerr_status_listener mc;
    boost::shared_ptr<receiver_t> receiver(new receiver_t(nh, mc));
    receiver->bind(12345);
    receiver->run();
    return 0;
}
