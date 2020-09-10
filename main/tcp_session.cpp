#include "tcp_session.h"

tcp_session::tcp_session(asio::ip::tcp::socket socket, std::function<void()> _onSocketError, std::function<void(uint8_t *, std::size_t)> _dataAvailable)
    : socket_(std::move(socket))
{
    OnSocketError = _onSocketError;
    DataAvailable = _dataAvailable;
}

tcp_session::~tcp_session()
{
    ESP_LOGI("Session", "Destroyed session");
}

void tcp_session::start()
{
    do_read();
}

void tcp_session::send(uint8_t *data, int length)
{
    //auto self(shared_from_this());
    asio::write(socket_, asio::buffer(data, length));
    /*asio::async_write(socket_, asio::buffer(data, length),
        [this, self](std::error_code ec, std::size_t length_)
        {
          if (!ec)
          {
            ESP_LOGI("SEND SOCKET", "Sent %d bytes", length_);
          }
          else
            ESP_LOGI("SEND SOCKET", "Send Socket problem %d", length_);
        });*/
}

void tcp_session::do_read()
{
    auto self(shared_from_this());
    socket_.async_read_some(asio::buffer(data_, max_length),
                            [this, self](std::error_code ec, std::size_t length) {
                                if (!ec)
                                {
                                    data_[length] = 0;
                                    DataAvailable(data_, length);

                                    do_read();
                                }
                                else
                                {
                                    ESP_LOGI("READ SOCKET", "Error");
                                    OnSocketError();
                                }
                            });
}