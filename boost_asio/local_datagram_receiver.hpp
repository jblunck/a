#ifndef __LOCAL_DATAGRAM_RECEIVER_HPP__
#define __LOCAL_DATAGRAM_RECEIVER_HPP__
#include "datagram_receiver.hpp"

// A reference-counted non-modifiable buffer class.
class shared_offset_buffer
{
public:
    // Construct from a shared buffer
    explicit shared_offset_buffer(boost::shared_ptr<std::vector<char> >& data,
                                  std::size_t offset,
                                  std::size_t length)
        : data_(data),
          buffer_(boost::asio::buffer(&(*data_)[offset], length))
    {
    }

    // Implement the ConstBufferSequence requirements.
    typedef boost::asio::const_buffer value_type;
    typedef const boost::asio::const_buffer* const_iterator;
    const boost::asio::const_buffer* begin() const { return &buffer_; }
    const boost::asio::const_buffer* end() const { return &buffer_ + 1; }

private:
    boost::shared_ptr<std::vector<char> > data_;
    boost::asio::const_buffer buffer_;
};

template <class T>
class buffered_datagram_writer
    : public boost::enable_shared_from_this<buffered_datagram_writer<T> >,
      public T
{
    boost::asio::local::datagram_protocol::socket _socket;
    const unsigned short _buffer_id;
    boost::shared_ptr<std::vector<char> > _data;

public:
    buffered_datagram_writer(boost::asio::io_service& io_service,
                             const unsigned short buffer_id)
        : _socket(io_service),
          _buffer_id(buffer_id)
    {
    }

    boost::asio::local::datagram_protocol::socket& socket()
    {
        return _socket;
    }

    // called to kick-off the sending of datagrams
    void start()
    {
        T::fill_buffer(_buffer_id, _data);

        // kick off first write
        boost::system::error_code ec;
        handle_write(ec, 0);
    }

    /*
     * Following member functions need to be implemented by <T> class:
     */

    // called once to initially fill the internal buffers with data to send
    void fill_buffer(const unsigned short buffer_id,
                     boost::shared_ptr<std::vector<char> >& data);

    // get size of next message starting at offset
    std::size_t get_size(const char * data);
    // get offset to next payload for message starting at offset
    std::size_t get_offset(const char * data);

private:
    // called when our write finished
    void handle_write(const boost::system::error_code& ec, std::size_t offset)
    {
        std::size_t msg_size = T::get_size(&(*_data)[offset]);
        if (msg_size <= 0) {
            /*
             * This is the end my friend: lets stop the io_service() so that
             * we do not wait in run() infinitely. But poll() all ready work
             * first.
             */
            _socket.cancel();
            _socket.close();
            _socket.get_io_service().poll();
            _socket.get_io_service().stop();
            return;
        }

        std::size_t msg_offset = T::get_offset(&(*_data)[offset]);
        shared_offset_buffer buffer(_data, offset + msg_offset, msg_size);
        _socket.async_send(buffer,
                           boost::bind(&buffered_datagram_writer::handle_write,
                                       boost::enable_shared_from_this<buffered_datagram_writer<T> >::shared_from_this(),
                                       boost::asio::placeholders::error,
                                       offset + msg_offset + msg_size));
    }
};

template <class EventHandler,
	  class StatusListener,
          class T>
class local_datagram_receiver :
    public datagram_receiver<EventHandler,
                             StatusListener,
                             boost::asio::local::datagram_protocol>
{
    typedef datagram_receiver<EventHandler,StatusListener,boost::asio::local::datagram_protocol> base_type;
    boost::asio::local::datagram_protocol::socket _socket;
    unsigned short _bind_port;

public:
    local_datagram_receiver(EventHandler& handler,
			  StatusListener& listener) :
        base_type(handler, listener),
	_socket(base_type::get_io_service()),
        _bind_port(0)
    {
    }

    local_datagram_receiver(EventHandler& handler,
                            StatusListener& listener,
                            const char * /* interface_address */,
                            const bool /* is_loopback */) :
        base_type(handler, listener),
	_socket(base_type::get_io_service()),
        _bind_port(0)
    {
    }

    /*
     * \exception std::runtime_error
     */
    void bind(const unsigned short port)
    {
        if (_bind_port)
            throw std::runtime_error("Already bound to port " + _bind_port);
        _bind_port = port;

        boost::shared_ptr<buffered_datagram_writer<T> > reader;
        reader.reset(new buffered_datagram_writer<T>(base_type::get_io_service(),
                                                     port));

	boost::system::error_code ec;
        boost::asio::local::connect_pair(_socket, reader->socket(), ec);
	if (ec)
	    throw std::runtime_error(ec.message());

        reader->start();

	// start receiving function
	base_type::start_receive(_socket);
    }

    void unbind(const unsigned short port)
    {
        if (port != _bind_port)
            throw std::runtime_error("Not bound to port " + port);

        _socket.cancel();
        _socket.close();
    }
};

#endif // __LOCAL_DATAGRAM_RECEIVER_HPP__
