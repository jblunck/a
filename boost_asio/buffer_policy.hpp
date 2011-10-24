#ifndef __BUFFER_POLICY_HPP__
#define __BUFFER_POLICY_HPP__

#include <boost/array.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/shared_ptr.hpp>

/*
 * do_handle<new_buffer_policy>:
 *
 * prepare_buffer {
 *   return new buffer;
 * }
 * start_receive(new_buffer);
 * do_handle(old_buffer);
 * consume_buffer(old_buffer) {
 *   delete old_buffer;
 *   return NULL;
 * }
 * 
 * do_handle<static_buffer_policy>:
 * 
 * prepare_buffer {
 *   // do nothing
 * }
 * do_handle(old_buffer)
 * consumer_buffer(old_buffer) {
 *   return old_buffer;
 * }
 */

template <std::size_t size>
class shared_buffer_policy
{
  class shared_buffer
  {
  public:
    explicit shared_buffer(boost::array<char,size>* data)
      : _data(data),
	_buffer(_data->c_array(), size)
    {
    }

    typedef boost::asio::mutable_buffer value_type;
    typedef const boost::asio::mutable_buffer* iterator;
    const boost::asio::mutable_buffer* begin() const { return &_buffer; }
    const boost::asio::mutable_buffer* end() const { return &_buffer + 1; }

  private:
    boost::shared_ptr<boost::array<char,size> > _data;
    boost::asio::mutable_buffer _buffer;
  };

public:
  typedef shared_buffer_policy::shared_buffer data_type;

  data_type operator()()
  {
    return data_type(new boost::array<char,size>());
  }
};

template <std::size_t size>
class static_buffer_policy
{
  boost::array<char,size> _data;

public:
  typedef boost::asio::mutable_buffers_1 data_type;

  data_type operator()()
  {
    return data_type(_data.c_array(), size);
  }
};

#endif // __BUFFER_POLICY_HPP__
