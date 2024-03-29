#include "local_datagram_receiver.hpp"
#include "read_from.hpp"
#include "datagram_receiver_handler.hpp"

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

BOOST_AUTO_TEST_CASE(read_from_string__fill_buffer)
{
    read_from_string h;
    boost::shared_ptr<std::vector<char> > data;
    h.fill_buffer(12345, data);

    std::string result(data->begin(), data->end());
    BOOST_CHECK_EQUAL(result, read_from_string::buf);
}

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

BOOST_AUTO_TEST_CASE(local_datagram_receiver__test1)
{
    struct LastReceivedBufferHandler nh;
    struct cerr_status_listener mc;
    boost::shared_ptr<receiver_t> receiver(new receiver_t(nh, mc));
    receiver->bind(12345);
    receiver->run();
    BOOST_CHECK_EQUAL(nh.get_buffer(), std::string(read_from_string::buf,4,10));
}

typedef local_datagram_receiver<struct nop_handler,
                                struct cerr_status_listener,
                                struct read_from_string> receiver2_t;

BOOST_AUTO_TEST_CASE(local_datagram_receiver__bind_throws)
{
    struct nop_handler nh;
    struct cerr_status_listener mc;
    boost::shared_ptr<receiver2_t> receiver(new receiver2_t(nh, mc));
    receiver->bind(12345);
    BOOST_CHECK_THROW(receiver->bind(12345), std::runtime_error);
    BOOST_CHECK_THROW(receiver->unbind(1), std::runtime_error);
}
