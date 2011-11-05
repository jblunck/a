#ifndef __DATAGRAM_RECEIVER_HANDLER_HPP__
#define __DATAGRAM_RECEIVER_HANDLER_HPP__

#include <iostream>

struct nop_handler
{
    typedef char * data_type;

    nop_handler()
    {
    }

    void operator()(data_type buffer, std::size_t length)
    {
        // for benchmark do not optimize this away
        __asm__ __volatile__("" : : : "memory");
    }
};

struct cerr_status_listener
{
    void error(const std::string & msg)
    {
        std::cerr << "Connection error: " << msg << std::endl;
    }
};

#endif // __DATAGRAM_RECEIVER_HANDLER_HPP__
