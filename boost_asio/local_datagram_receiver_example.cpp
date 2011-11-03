#include "read_from.hpp"
#include "local_datagram_receiver.hpp"
#include "datagram_receiver_handler.hpp"
#include <iostream>

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

typedef local_datagram_receiver<struct LastReceivedBufferHandler,
                                struct cerr_status_listener,
                                struct read_from_string> receiver_t;

int main()
{
    struct LastReceivedBufferHandler nh;
    struct cerr_status_listener mc;
    boost::shared_ptr<receiver_t> receiver(new receiver_t(nh, mc));
    receiver->bind(12345);
    receiver->run();

    std::cout << "Received: " << nh.get_buffer() << std::endl;
    return 0;
}
