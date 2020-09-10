#ifndef _TCP_SESSION_H_
#define _TCP_SESSION_H_

#include <functional>
#include "asio.hpp"

class tcp_session : public std::enable_shared_from_this<tcp_session>
{
public:
  tcp_session(asio::ip::tcp::socket socket, std::function<void()> _onSocketError, std::function<void(uint8_t *, std::size_t)> _dataAvailable);
  ~tcp_session();

  void start();
  void send(uint8_t* data, int length);

private:
  void do_read();
  std::function<void()> OnSocketError;
  std::function<void(uint8_t *, std::size_t)> DataAvailable;

  asio::ip::tcp::socket socket_;

  enum { max_length = 1024 };
  uint8_t data_[max_length];
  
};

#endif