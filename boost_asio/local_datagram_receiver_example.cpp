#include "local_datagram_receiver.hpp"
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

struct read_from_file
{
    // called to start reading the file
    void get_buffer(const std::string& filename,
                    boost::shared_ptr<std::vector<char> >& data)
    {
        std::ifstream file(filename.c_str(),
                           std::ios::in|std::ios::binary|std::ios::ate);
        if (!file.is_open())
            throw std::runtime_error("Can not open file: " + filename);

        std::size_t size = file.tellg();
        std::vector<char> *buf = new std::vector<char>(size);
        file.seekg (0, std::ios::beg);
        file.read (&(*buf)[0], size);
        file.close();
        data.reset(buf);
    }

    std::size_t get_size(const char * data)
    {
        std::size_t size;

        size = data[3] - '0';
        size += (data[2] - '0') * 10;
        size += (data[1] - '0') * 100;
        size += (data[0] - '0') * 1000;

        std::cerr << __FUNCTION__ << ": " << size << std::endl;
        return size;
    }
};

typedef local_datagram_receiver<NullHandler,
                                struct my_connection,
                                struct read_from_file> receiver_t;

int main()
{
    struct NullHandler nh;
    struct my_connection mc;
    boost::shared_ptr<receiver_t> receiver(new receiver_t(nh, mc));
    receiver->bind(12345);
    receiver->run();
    return 0;
}
