#include "local_datagram_receiver.hpp"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <string>

using boost::asio::detail::buffer_cast_helper;
using boost::asio::detail::buffer_size_helper;

BOOST_AUTO_TEST_CASE(shared_offset_buffer_test1)
{
    std::string data_string("foo.dat");
    boost::shared_ptr<std::vector<char> > data;
    data.reset(new std::vector<char>(data_string.begin(), data_string.end()));

    shared_offset_buffer b(data, 3, 4);
    void *p1 = const_cast<void *>(buffer_cast_helper(*b.begin()));
    BOOST_CHECK(p1 != &((*data)[0]));
    BOOST_CHECK(p1 == &((*data)[3]));
}

shared_offset_buffer get_test_buffer(void **result_ptr)
{
    std::string data_string("foo.dat");
    boost::shared_ptr<std::vector<char> > data;
    data.reset(new std::vector<char>(data_string.begin(), data_string.end()));

    *result_ptr = &((*data)[3]);
    return shared_offset_buffer(data, 3, 4);
}

BOOST_AUTO_TEST_CASE(shared_offset_buffer_test2)
{
    void *result;
    shared_offset_buffer b(get_test_buffer(&result));
    std::string data_string("foo.dat");

    void *p = const_cast<void *>(buffer_cast_helper(*b.begin()));
    BOOST_CHECK(p == result);

    std::string result_string(static_cast<const char *>(buffer_cast_helper(*b.begin())),
                              buffer_size_helper(*b.begin()));
    if (data_string == result_string)
        BOOST_ERROR(data_string + " == " + result_string);
    BOOST_CHECK_EQUAL(std::string(data_string,3,4), result_string);
}