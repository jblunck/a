#ifndef __LOCAL_DATAGRAM_RECEIVER_HPP__
#define __LOCAL_DATAGRAM_RECEIVER_HPP__
#include "datagram_receiver.hpp"

#include <boost/lexical_cast.hpp>
#include <fstream>

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
class file_reader_handler
    : public boost::enable_shared_from_this<file_reader_handler<T> >,
      public T
{
    boost::asio::local::datagram_protocol::socket _socket;
    const std::string _filename;
    boost::shared_ptr<std::vector<char> > _data;

public:
    file_reader_handler(boost::asio::io_service& io_service,
                        const std::string& filename)
        : _socket(io_service),
          _filename(filename)
    {
    }

    boost::asio::local::datagram_protocol::socket& socket()
    {
        return _socket;
    }

    // called to start reading the file
    void start()
    {
        T::get_buffer(_filename, _data);

        // kick off first write
        boost::system::error_code ec;
        handle_write(ec, 0);
    }

    // called to start reading the file
    void get_buffer(const std::string& filename,
                    boost::shared_ptr<std::vector<char> >& data);

    std::size_t get_size(const char * data);

private:
    // called when our write finished
    void handle_write(const boost::system::error_code& ec, std::size_t offset)
    {
        std::size_t size = T::get_size(&(*_data)[offset]);
        if (size <= 0) {
            /*
             * This is the end my friend: lets stop the io_service() so that
             * we do not wait in run() infinitely.
             */
            _socket.get_io_service().stop();
            return;
        }

        shared_offset_buffer buffer(_data, offset+4, size);
        _socket.async_send(buffer,
                           boost::bind(&file_reader_handler::handle_write,
                                       boost::enable_shared_from_this<file_reader_handler<T> >::shared_from_this(),
                                       boost::asio::placeholders::error,
                                       offset+4+size));
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

public:
    local_datagram_receiver(EventHandler& handler,
			  StatusListener& listener) :
        base_type(handler, listener),
	_socket(base_type::get_io_service())
    {
    }

    local_datagram_receiver(EventHandler& handler,
                            StatusListener& listener,
                            const char * /* interface_address */,
                            const bool /* is_loopback */) :
        base_type(handler, listener),
	_socket(base_type::get_io_service())
    {
    }

    /*
     * \exception std::runtime_error
     */
    void bind(const unsigned short port)
    {
        // open new socket for reading in test file
        boost::shared_ptr<file_reader_handler<T> > reader;
        reader.reset(new file_reader_handler<T>(base_type::get_io_service(),
                                             boost::lexical_cast<std::string>(port) + ".dat"));

	boost::system::error_code ec;
        boost::asio::local::connect_pair(_socket, reader->socket(), ec);
	if (ec)
	    throw std::runtime_error(ec.message());

        reader->start();

	// start receiving function
	base_type::start_receive(_socket);
    }

};

#endif // __LOCAL_DATAGRAM_RECEIVER_HPP__
