#include "commands.h"
#include "storage_keys.h"
#include <stdio.h>
#include <string.h>
#include "constants.h"

void commands::register_commands()
{
    register_uart_commands();
    register_wifi_commands();
    register_reboot_command();
    register_clear_nvs_commands();
}

// UART
int commands::uart_configure_command(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &uart_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, uart_args.end, argv[0]);
        return 1;
    }

    // Uart number
    int uart_num = uart_args.uart->ival[0];
    if (uart_num > 2)
    {
        printf("Uart number is invalid, please insert 0, 1 or 2");
        return 1;
    }
    
    // Aux vars
    char STORAGE_KEY[50];
    int32_t aux_int = 0;
    char aux_str[100];

    // ENABLE
    sprintf(STORAGE_KEY, STORAGE_UART_ENABLE, uart_num);
    if (uart_args.enable->count == 0) {
        uart_args.enable->ival[0] = storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &aux_int) == ESP_OK ? aux_int : UART_DEFAULT_ENABLE[uart_num];
    }
    storage::write_int32(STORAGE_NAMESPACE, STORAGE_KEY, uart_args.enable->ival[0]);

    // BAUDS
    sprintf(STORAGE_KEY, STORAGE_UART_BAUDS, uart_num);
    if (uart_args.bauds->count == 0) {
        uart_args.bauds->ival[0] = storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &aux_int) == ESP_OK ? aux_int : UART_DEFAULT_BAUDS;
    }
    storage::write_int32(STORAGE_NAMESPACE, STORAGE_KEY, uart_args.bauds->ival[0]);

    // TCP Port
    sprintf(STORAGE_KEY, STORAGE_UART_TCP_PORT, uart_num);
    if (uart_args.tcp_port->count == 0) {
        uart_args.tcp_port->ival[0] = storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &aux_int) == ESP_OK ? aux_int : UART_DEFAULT_TCP_PORT[uart_num];
    }
    storage::write_int32(STORAGE_NAMESPACE, STORAGE_KEY, uart_args.tcp_port->ival[0]);

    // TX_PIN
    sprintf(STORAGE_KEY, STORAGE_UART_TX_PIN, uart_num);
    if (uart_args.tx_pin->count == 0) {
        uart_args.tx_pin->ival[0] = storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &aux_int) == ESP_OK ? aux_int : UART_DEFAULT_TX_PIN[uart_num];
    }
    storage::write_int32(STORAGE_NAMESPACE, STORAGE_KEY, uart_args.tx_pin->ival[0]);

    // RX_PIN
    sprintf(STORAGE_KEY, STORAGE_UART_RX_PIN, uart_num);
    if (uart_args.rx_pin->count == 0) {
        uart_args.rx_pin->ival[0] = storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &aux_int) == ESP_OK ? aux_int : UART_DEFAULT_RX_PIN[uart_num];
    }
    storage::write_int32(STORAGE_NAMESPACE, STORAGE_KEY, uart_args.rx_pin->ival[0]);
    
    // TX_BUFFER
    sprintf(STORAGE_KEY, STORAGE_UART_TX_BUFFER, uart_num);
    if (uart_args.tx_buffer->count == 0) {
        uart_args.tx_buffer->ival[0] = storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &aux_int) == ESP_OK ? aux_int : UART_DEFAULT_BUFFER;
    }
    storage::write_int32(STORAGE_NAMESPACE, STORAGE_KEY, uart_args.tx_buffer->ival[0]);

    // RX_BUFFER
    sprintf(STORAGE_KEY, STORAGE_UART_RX_BUFFER, uart_num);
    if (uart_args.rx_buffer->count == 0) {
        uart_args.rx_buffer->ival[0] = storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &aux_int) == ESP_OK ? aux_int : UART_DEFAULT_BUFFER;
    }
    storage::write_int32(STORAGE_NAMESPACE, STORAGE_KEY, uart_args.rx_buffer->ival[0]);

    // DATA_BITS
    sprintf(STORAGE_KEY, STORAGE_UART_DATA_BITS, uart_num);
    if (uart_args.data_bits->count == 0) {
        uart_args.data_bits->ival[0] = storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &aux_int) == ESP_OK ? aux_int : 8;
    }
    storage::write_int32(STORAGE_NAMESPACE, STORAGE_KEY, uart_args.data_bits->ival[0]);

    // PARITY
    sprintf(STORAGE_KEY, STORAGE_UART_PARITY, uart_num);
    if (uart_args.parity->count == 0) {
        uart_args.parity->ival[0] = storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &aux_int) == ESP_OK ? aux_int : UART_PARITY_NONE;
    }
    storage::write_int32(STORAGE_NAMESPACE, STORAGE_KEY, uart_args.parity->ival[0]);

    // STOP_BITS
    sprintf(STORAGE_KEY, STORAGE_UART_STOP_BITS, uart_num);
    if (uart_args.stop_bits->count == 0) {
        uart_args.stop_bits->ival[0] = storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &aux_int) == ESP_OK ? aux_int : 1;
    }
    storage::write_int32(STORAGE_NAMESPACE, STORAGE_KEY, uart_args.stop_bits->ival[0]);
    
    return 0;
}

void commands::register_uart_commands()
{
    uart_args.uart = arg_int1(NULL, NULL, "<0|1|2>", "Uart number, 0, 1, 2");
    uart_args.enable = arg_int1(NULL, NULL, "<enable=1|disable=0>", "Enable or disable port (enable)");
    uart_args.bauds = arg_int1(NULL, NULL, "<bauds>", "Baud rate (115200)");
    uart_args.tcp_port = arg_int0(NULL, "tcp_port", "<tcp_port>", "Listening TCP Port");
    uart_args.tx_pin = arg_int0(NULL, "tx_pin", "<tx_pin>", "TX Pin");
    uart_args.rx_pin = arg_int0(NULL, "rx_pin", "<rx_pin>", "RX Pin");
    uart_args.tx_buffer = arg_int0(NULL, "tx_buffer", "<tx_buffer>", "Transmission buffer in bytes (2048)");
    uart_args.rx_buffer = arg_int0(NULL, "rx_buffer", "<rx_buffer>", "Reception buffer in bytes (2048)");
    uart_args.data_bits = arg_int0(NULL, "data_bits", "<data_bits>", "Number of data bits (8)");
    uart_args.parity = arg_int0(NULL, "parity", "<odd=3|even=2|none=0>", "Parity (none)");
    uart_args.stop_bits = arg_int0(NULL, "stop_bits", "<stop_bits>", "Number of stop bits (1)");
    uart_args.end = arg_end(8);

    static esp_console_cmd_t uart_config_cmd = {
        .command = "uart_config",
        .help = "Set uart parameters",
        .hint = NULL,
        .func = &uart_configure_command,
        .argtable = &uart_args
    };

    esp_console_cmd_register(&uart_config_cmd);
}

// WIFI
int commands::wifi_configure_command(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &wifi_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, wifi_args.end, argv[0]);
        return 1;
    }

    if (strlen(wifi_args.password->sval[0]) < 8)
    {
        printf("Wifi password cannot be less than 8 chars");
        return 1;
    }

    int32_t aux_int = 0;

    // WIFI Mode
    if (wifi_args.mode->count == 0) {
        wifi_args.mode->ival[0] = storage::read_int32(STORAGE_NAMESPACE, STORAGE_WIFI_MODE, &aux_int) == ESP_OK ? aux_int : 0;
    }
    storage::write_int32(STORAGE_NAMESPACE, STORAGE_WIFI_MODE, wifi_args.mode->ival[0]);

    // SSID
    storage::write_string(STORAGE_NAMESPACE, STORAGE_WIFI_SSID, wifi_args.ssid->sval[0]);

    // Password
    storage::write_string(STORAGE_NAMESPACE, STORAGE_WIFI_PASSWD, wifi_args.password->sval[0]);

    // WIFI channel
    if (wifi_args.channel->count == 0) {
        wifi_args.channel->ival[0] = storage::read_int32(STORAGE_NAMESPACE, STORAGE_WIFI_MODE, &aux_int) == ESP_OK ? aux_int : 6;
    }
    storage::write_int32(STORAGE_NAMESPACE, STORAGE_WIFI_MODE, wifi_args.channel->ival[0]);

    return 0;
}

void commands::register_wifi_commands()
{
    wifi_args.mode = arg_int1(NULL, NULL, "<ap=0|station=1>", "Wifi mode. AP or Station(client)");
    wifi_args.ssid = arg_str1(NULL, NULL, "<ssid>", "SSID of the wifi network");
    wifi_args.password = arg_str1(NULL, NULL, "<passwd>", "Wifi password. If no password, put \"\"");
    wifi_args.channel = arg_int0(NULL, "channel", "<channel>", "Wifi channel in AP mode");
    wifi_args.end = arg_end(2);

    static esp_console_cmd_t wifi_config_cmd = {
        .command = "wifi_config",
        .help = "Set wifi parameters",
        .hint = NULL,
        .func = &wifi_configure_command,
        .argtable = &wifi_args
    };

    esp_console_cmd_register(&wifi_config_cmd);
}

// Reboot
int commands::reboot_command(int argc, char **argv)
{
    esp_restart();
}

void commands::register_reboot_command()
{
    const esp_console_cmd_t cmd = {
        .command = "reboot",
        .help = "Reboot",
        .hint = NULL,
        .func = &reboot_command,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

// Factory settings
int commands::clear_nvs_command(int argc, char **argv)
{
    // Format NVS
    storage::format_nvs();

    // Reboot
    esp_restart();
}

void commands::register_clear_nvs_commands()
{
    const esp_console_cmd_t cmd = {
        .command = "factory",
        .help = "Restore factory settings. Board will reboot",
        .hint = NULL,
        .func = &clear_nvs_command,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}


