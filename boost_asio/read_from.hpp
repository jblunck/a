#ifndef __READ_FROM_HPP__
#define __READ_FROM_HPP__

#include <boost/shared_ptr.hpp>
#include <vector>

struct read_from_file
{
    void fill_buffer(const unsigned short port,
                     boost::shared_ptr<std::vector<char> >& data);

    std::size_t get_size(const char * data);

    std::size_t get_offset(const char * data);
};

struct read_from_string
{
    static std::string buf;

    void fill_buffer(const unsigned short port,
                     boost::shared_ptr<std::vector<char> >& data);

    std::size_t get_size(const char * data);

    std::size_t get_offset(const char * data);
};

#endif // __READ_FROM_HPP__
