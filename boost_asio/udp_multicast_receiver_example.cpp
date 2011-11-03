#include "udp_multicast_receiver.hpp"
#include "datagram_receiver_handler.hpp"
#include <iostream>

typedef udp_multicast_receiver<struct nop_handler,
                               struct cerr_status_listener> receiver_t;

int main()
{
    struct nop_handler nh;
    struct cerr_status_listener mc;
    boost::shared_ptr<receiver_t> receiver(new receiver_t(nh, mc,
                                                          "127.0.0.1", true));
    receiver->join("239.1.2.3", 12345);
    receiver->leave("239.1.2.3", 12345);
    receiver->run();
    return 0;
}
