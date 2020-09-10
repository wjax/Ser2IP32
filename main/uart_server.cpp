#include <sstream>
#include <string>
#include "uart_server.h"

uart_server::uart_server(asio::io_context *io_context, short port, uart_port_t uart, LED_Display *dis, int rx_led, int tx_led, int sessionConnectedLed)
    //: acceptor_(io_context/*, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)*/)
{
    // asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
    // acceptor_.open(endpoint.protocol());
    // acceptor_.bind(endpoint);
    // acceptor_.listen(0);

    _io_context = io_context;
    _port = port;
    // LED Matrix
    _dis = dis;
    _rxLed = rx_led;
    _txLed = tx_led;
    _sessionConnectedLed = sessionConnectedLed;
    // Start listening socket
    do_accept();
    // Uart
    _uart = uart;
    std::stringstream ss;
    ss << "uart_rx_task" << port;
    xTaskCreate(this->start_uart_impl, ss.str().c_str(), 1024 * 4, this, configMAX_PRIORITIES, NULL);
    //ss << "io_service";
    //xTaskCreate(this->start_asio, ss.str().c_str(), 1024 * 4, this, configMAX_PRIORITIES, NULL);
}

uart_server::~uart_server()
{
}

void uart_server::do_accept()
{
    auto acceptor = std::make_shared<asio::ip::tcp::acceptor>(*_io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), _port));
    acceptor->async_accept(
        [this, acceptor](std::error_code ec, asio::ip::tcp::socket socket) {
            if (!ec)
            {
                _dis->drawpixB(_sessionConnectedLed, BLUE);
                p_session = std::make_shared<tcp_session>(std::move(socket),
                    [=]() {
                        this->onsocket_disconection();
                    },
                    [=](uint8_t * data, std::size_t length) {
                        this->data_available(data, length);
                    });
                p_session->start();

                //acceptor_.cancel();
            }
            else
                 ESP_LOGI("Acceptor", "Error");

            
        });
}

void uart_server::onsocket_disconection()
{
    ESP_LOGI("UART Server", "On Socket Disconnection");
    _dis->drawpixB(_sessionConnectedLed, 0x000000);
    p_session = nullptr;
    do_accept();
}

void uart_server::data_available(uint8_t * data, std::size_t length)
{
    _dis->drawpixB(_txLed, RED, (uint8_t)2);

    uart_write_bytes(_uart, (const char *)data, length);
}

void uart_server::start_asio(void *_this)
{
    ((uart_server *)_this)->start_asio();
}

void uart_server::start_asio()
{
    //_io_context.run();
}

void uart_server::start_uart_impl(void *_this)
{
    ((uart_server *)_this)->start_uart();
}

void uart_server::start_uart()
{
    ESP_LOGI("START UART", "START");
    uint8_t data[RX_BUF_SIZE];// = (uint8_t *)malloc(RX_BUF_SIZE);
    //uint8_t data[RX_BUF_SIZE + 1];
    while (1)
    {

        const int rxBytes = uart_read_bytes(_uart, data, RX_BUF_SIZE, 10 / portTICK_RATE_MS);
        if (rxBytes > 0)
        {
            _dis->drawpixB(_rxLed, GREEN, (uint8_t)2);
            // Send over session if available
            if (p_session)
                p_session->send(data, rxBytes);
        }
    }
    //free(data);
}