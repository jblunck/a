#include "local_datagram_receiver.hpp"
#include "read_from.hpp"

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

struct LastReceivedBufferHandler
{
    typedef char * data_type;
    std::string _buffer;
    bool _has_received;

    LastReceivedBufferHandler()
        : _has_received(false)
    {
    }

    void operator()(data_type buffer, std::size_t length)
    {
        _buffer = buffer;
        _has_received = true;
    }

    const std::string& get_buffer()
    {
        if (!_has_received)
            throw std::runtime_error("Not received anything");
        return _buffer;
    }
};

typedef local_datagram_receiver<LastReceivedBufferHandler,
                                struct my_connection,
                                struct read_from_string> receiver_t;

int main()
{
    struct LastReceivedBufferHandler nh;
    struct my_connection mc;
    boost::shared_ptr<receiver_t> receiver(new receiver_t(nh, mc));
    receiver->bind(12345);
    receiver->run();

    std::cout << "Received: " << nh.get_buffer() << std::endl;
    return 0;
}
