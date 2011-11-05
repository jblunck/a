#include "read_from.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <iostream>

// called to start reading the file
void read_from_file::fill_buffer(const unsigned short port,
                                 boost::shared_ptr<std::vector<char> >& data)
{
    const std::string filename(boost::lexical_cast<std::string>(port) +
                               ".dat");
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

std::size_t read_from_file::get_size(const char * data)
{
    std::size_t size;

    size = data[3] - '0';
    size += (data[2] - '0') * 10;
    size += (data[1] - '0') * 100;
    size += (data[0] - '0') * 1000;

    std::cerr << __FUNCTION__ << ": " << size << std::endl;
    return size;
}

std::size_t read_from_file::get_offset(const char * data)
{
    return 4;
}

std::string read_from_string::buf("001012345678900000");

// called to start reading the file
void read_from_string::fill_buffer(const unsigned short port,
                                   boost::shared_ptr<std::vector<char> >& data)
{
    data.reset(new std::vector<char>(buf.begin(),buf.end()));
}

std::size_t read_from_string::get_size(const char * data)
{
    std::size_t size;

    size = data[3] - '0';
    size += (data[2] - '0') * 10;
    size += (data[1] - '0') * 100;
    size += (data[0] - '0') * 1000;
    return size;
}

std::size_t read_from_string::get_offset(const char * data)
{
    return 4;
}
