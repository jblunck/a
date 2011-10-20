#include "udp_datagram_receiver.hpp"
#include <iostream>

#define BUFFER_SIZE 1024

struct NullHandler
{
  typedef char * data_type;

  static size_t data_type_size()
  {
    return BUFFER_SIZE;
  }

  NullHandler()
  {
  }

    void operator()(data_type buffer, std::size_t length)
  {
    // for benchmark do not optimize this away
    __asm__ __volatile__("" : : : "memory");
    std::cout << "Received " << length << " bytes" << std::endl;
  }

  data_type get_next(data_type prev)
  {
    return static_cast<data_type>(buffer);
  }

    char buffer[BUFFER_SIZE];
};

struct my_connection
{
  void error(const std::string & msg)
  {
      std::cerr << "Connection error: " << msg << std::endl;
  }
};

int main()
{
  struct NullHandler nh;
  struct my_connection mc;
  udp_datagram_receiver<NullHandler,struct my_connection> receiver(nh, mc);
  receiver.bind(12345);
  receiver.run();
  return 0;
}
