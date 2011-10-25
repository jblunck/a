/*
 * echo "TEST" | socat - UDP-DATAGRAM:127.0.0.1:12345
 */

#include "udp_datagram_receiver.hpp"
#include <iostream>

struct NullHandler
{
    typedef char * data_type;

    NullHandler()
    {
    }

    void operator()(data_type buffer, std::size_t length)
    {
        // for benchmark do not optimize this away
        __asm__ __volatile__("" : : : "memory");
        std::cout << "Received " << length << " bytes" << std::endl;
    }
};

struct my_connection
{
    void error(const std::string & msg)
    {
        std::cerr << "Connection error: " << msg << std::endl;
    }
};

typedef udp_datagram_receiver<NullHandler,struct my_connection> receiver_t;

int main()
{
    struct NullHandler nh;
    struct my_connection mc;
    boost::shared_ptr<receiver_t> receiver(new receiver_t(nh, mc));
    receiver->bind(12345);
    receiver->run();
    return 0;
}
