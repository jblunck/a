#include "read_from.hpp"
#include "local_datagram_receiver.hpp"
#include "datagram_receiver_handler.hpp"
#include "../stop_watch.hpp"
#include <iostream>

template <typename T>
struct one_second_policy : always_timeout_policy<T, 1>
{
    one_second_policy(boost::asio::basic_datagram_socket<T> & socket)
        : always_timeout_policy<T, 1>(socket)
    {
    }
};

typedef local_datagram_receiver<struct read_from_string,
                                struct nop_handler,
                                struct cerr_status_listener,
                                one_second_policy> receiver_t;

#define MESSAGES_NR 1e6

int main()
{
    read_from_string::buf = "";
    for (int i=0 ; i < MESSAGES_NR ; ++i) {
        read_from_string::buf.append("00101234567890");
    }
    read_from_string::buf.append("0000");

    struct nop_handler nh;
    struct cerr_status_listener mc;
    boost::shared_ptr<receiver_t> receiver(new receiver_t(nh, mc));
    receiver->bind(12345);

    stop_watch s;
    receiver->run();
    uint64_t t = s.elapsed_ns();
    std::cout << ( MESSAGES_NR * 1e9 ) / t << " msg/s" << std::endl;

    // The following is necessary to free all used memory:
    // 1.) cancel the handler
    receiver->unbind(12345);

    // 2.) execute the handler to process cancelation
    receiver->get_io_service().reset();
    receiver->get_io_service().poll();
    return 0;
}
