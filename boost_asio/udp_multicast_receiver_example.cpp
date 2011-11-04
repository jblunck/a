#include "udp_multicast_receiver.hpp"
#include "datagram_receiver_handler.hpp"
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <unistd.h> // for getopt

typedef udp_multicast_receiver<struct nop_handler,
                               struct cerr_status_listener> receiver_t;

using namespace boost::asio::ip;

int main(int argc, char *argv[])
{
    // parse options
    address opts_interface_ipaddr(address_v4::any());
    address opts_ipaddr;
    unsigned short opts_port(0);

    int opt;
    while ((opt = getopt(argc, argv, "i:h:p:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            opts_interface_ipaddr = address::from_string(optarg);
            break;
        case 'h':
            opts_ipaddr = address::from_string(optarg);
            break;
        case 'p':
            opts_port = boost::lexical_cast<unsigned short>(optarg);
            break;
        }
    }

    struct nop_handler nh;
    struct cerr_status_listener mc;
    boost::shared_ptr<receiver_t> receiver(new receiver_t(nh, mc,
                                                          opts_interface_ipaddr.to_string().c_str(),
                                                          true));
    receiver->join(opts_ipaddr.to_string().c_str(), opts_port);
    receiver->run();
    receiver->leave(opts_ipaddr.to_string().c_str(), opts_port);
    return 0;
}
