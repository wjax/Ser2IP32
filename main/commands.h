#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "storage.h"

// Storage
#define STORAGE_NAMESPACE "storage"

static struct {
            struct arg_int *uart;
            struct arg_int *enable;
            struct arg_int *bauds;
            struct arg_int *tcp_port;
            struct arg_int *tx_pin;
            struct arg_int *rx_pin;
            struct arg_int *tx_buffer;
            struct arg_int *rx_buffer;
            struct arg_int *data_bits;
            struct arg_int *parity;
            struct arg_int *stop_bits;
            struct arg_end *end;
        } uart_args;

static struct {
            struct arg_int *mode;
            struct arg_str *ssid;
            struct arg_str *password;
            struct arg_int *channel;
            struct arg_end *end;
        } wifi_args;

class commands
{
    public:
        static void register_commands();

    private:
        
        // UART
        static void register_uart_commands();
        static int uart_configure_command(int argc, char **argv);
        // Wifi
        static void register_wifi_commands();
        static int wifi_configure_command(int argc, char **argv);
        // Reboot
        static void register_reboot_command();
        static int reboot_command(int argc, char **argv);
        // Clear NVS
        static void register_clear_nvs_commands();
        static int clear_nvs_command(int argc, char **argv);


};


#endif