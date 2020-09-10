#ifndef _UART_SERVER_H_
#define _UART_SERVER_H_

#include "LED_Display.h"
#include "tcp_session.h"
#include "driver/uart.h"

class uart_server
{
public:
  uart_server(asio::io_context* io_context, short port, uart_port_t uart, LED_Display *dis, int rx_led, int tx_led, int sessionConnectedLed);
  ~uart_server();

private:
  const int RX_BUF_SIZE = 1024;
  void do_accept();
  static void start_uart_impl(void *_this);
  void start_uart();
  static void start_asio(void *_this);
  void start_asio();
  void onsocket_disconection();
  void data_available(uint8_t *, std::size_t length);

  uart_port_t _uart;
  std::shared_ptr<tcp_session> p_session;
  std::shared_ptr<asio::ip::tcp::acceptor> acceptor_;
  int _port;
  asio::io_context *_io_context;

  LED_Display *_dis;
  int _rxLed;
  int _txLed;
  int _sessionConnectedLed;
};

#endif
