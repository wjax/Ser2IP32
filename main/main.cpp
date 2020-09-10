#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "asio.hpp"
#include "protocol_examples_common.h"
#include "esp_event.h"
#include "tcpip_adapter.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "uart_server.h"
#include "LED_Display.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <cstdio>

#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"

#include "storage.h"
#include "wifi.h"
#include "commands.h"
#include "constants.h"
#include "storage_keys.h"

const char *TAG = "SER2IP32";

void initNVS()
{
  esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void configure_uart(uart_port_t uartNum,
                      int bauds,
                      int tx_pin,
                      int rx_pin,
                      int buff_size_tx = UART_DEFAULT_BUFFER,
                      int buff_size_rx = UART_DEFAULT_BUFFER,
                      int data_bits = UART_DEFAULT_DATA_BITS,
                      uart_parity_t parity = (uart_parity_t)UART_DEFAULT_PARITY,
                      int stop_bits = UART_DEFAULT_STOP_BITS,
                      uart_hw_flowcontrol_t flow_control = UART_HW_FLOWCTRL_DISABLE)
{
  // Convert Data Bits
  uart_word_length_t uart_data_bits = UART_DATA_8_BITS;
  switch (data_bits)
  {
    case 5:
      uart_data_bits = UART_DATA_5_BITS; break;
    case 6:
      uart_data_bits = UART_DATA_6_BITS; break;
    case 7:
      uart_data_bits = UART_DATA_7_BITS; break;
    case 8:
      uart_data_bits = UART_DATA_8_BITS; break;
    default:
      uart_data_bits = UART_DATA_8_BITS; break;
  }

  // Convert Stop Bits
  uart_stop_bits_t uart_stop_bits = UART_STOP_BITS_1;
  switch (stop_bits)
  {
    case 1:
      uart_stop_bits = UART_STOP_BITS_1; break;
    case 2:
      uart_stop_bits = UART_STOP_BITS_2; break;
    default:
      uart_stop_bits = UART_STOP_BITS_1; break;
  }

  const uart_config_t uart_config = {
      .baud_rate = bauds,
      .data_bits = uart_data_bits,
      .parity = parity,
      .stop_bits = uart_stop_bits,
      .flow_ctrl = flow_control
  };
  uart_param_config(uartNum, &uart_config);
  uart_set_pin(uartNum, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  uart_driver_install(uartNum, buff_size_rx, buff_size_tx, 0, NULL, 0);
}

int wait_for_console_mode()
{
  // Configure UART0 for console
  configure_uart(UART_NUM_0, 115200, 1, 3);
  // Emit message to interrupt
  //uart_tx_chars(UART_NUM_0, INITIAL_CONFIGURE_MSG, strlen(INITIAL_CONFIGURE_MSG));
  int console_mode = 0;
  int ms_elapsed = 0;
  uint8_t *data = (uint8_t *)malloc(100 + 1);
  while (console_mode < 1 &&  ms_elapsed < CONSOLE_START_TIMEOUT)
  {
    const int rxBytes = uart_read_bytes(UART_NUM_0, data, 100, 100 / portTICK_RATE_MS);
    if (rxBytes > 0)
    console_mode = 1;
    //::vTaskDelay(ms/portTICK_PERIOD_MS);
    ms_elapsed += 100;
  }
  free(data);
  
  return console_mode;
}

static void initialize_console()
{
    /* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);

    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

    /* Configure UART. Note that REF_TICK is used so that the baud rate remains
     * correct while APB frequency is changing in light sleep mode.
     */
    const uart_config_t uart_config = {
            .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .use_ref_tick = true
    };
    ESP_ERROR_CHECK( uart_param_config(UART_NUM_0, &uart_config) );

    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(UART_NUM_0,
            256, 0, 0, NULL, 0) );

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(UART_NUM_0);

    /* Initialize the console */
    esp_console_config_t console_config = {
            .max_cmdline_length = 256,
            .max_cmdline_args = 12,
    };
    ESP_ERROR_CHECK( esp_console_init(&console_config) );

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);

    /* Set command history size */
    linenoiseHistorySetMaxLen(100);
}

void start_and_run_console()
{
    // Init Console
    initialize_console();
    // Help command
    esp_console_register_help_command();
    // Add Commands
    commands::register_commands();
    
    int exit = 0;
    const char* prompt = "Ser2IP32> ";

    printf("\n"
           "Welcome to Ser2IP32 configuration console\n"
           "Type 'help' to get the list of commands.\n"
           "Use UP/DOWN arrows to navigate through command history.\n"
           "Press TAB when typing command name to auto-complete.\n");

    /* Figure out if the terminal supports escape sequences */
    int probe_status = linenoiseProbe();
    if (probe_status) { /* zero indicates success */
        printf("\n"
               "Your terminal application does not support escape sequences.\n"
               "Line editing and history features are disabled.\n"
               "On Windows, try using Putty instead.\n");
        linenoiseSetDumbMode(1);
    }
    
    /* Main loop */
    while(!exit) {
        /* Get a line using linenoise.
         * The line is returned when ENTER is pressed.
         */
        char* line = linenoise(prompt);
        if (line == NULL) { /* Ignore empty lines */
            continue;
        }
        /* Add the command to the history */
        linenoiseHistoryAdd(line);

        /* Try to run the command */
        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND) {
            printf("Unrecognized command\n");
        } else if (err == ESP_ERR_INVALID_ARG) {
            // command was empty
        } else if (err == ESP_OK && ret != ESP_OK) {
            printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(err));
        } else if (err != ESP_OK) {
            printf("Internal error: %s\n", esp_err_to_name(err));
        }
        /* linenoise allocates line buffer on the heap, so need to free it */
        linenoiseFree(line);
    }
}

void start_wifi(LED_Display *dis)
{

  // Wifi Mode
  int mode = WIFI_MODE_AP;
  if (storage::read_int32(STORAGE_NAMESPACE, STORAGE_WIFI_MODE, &mode) != ESP_OK)
    mode = WIFI_MODE_AP; // AP

  dis->drawpixB(mode ? LED_DEFAULT_STATION : LED_DEFAULT_AP, WHITE); 

  // Wifi SSID
  char wifi_ssid[WIFI_SSID_MAX_LENGTH+1];
  size_t ssid_length = WIFI_SSID_MAX_LENGTH+1;
  if (storage::read_string(STORAGE_NAMESPACE, STORAGE_WIFI_SSID, wifi_ssid, &ssid_length) != ESP_OK)
    strlcpy(wifi_ssid, WIFI_AP_DEFAULT_SSID, WIFI_SSID_MAX_LENGTH+1);

  // Wifi Passwd
  char wifi_passwd[WIFI_PASSWD_MAX_LENGTH+1];
  size_t passwd_length = WIFI_PASSWD_MAX_LENGTH+1;
  if (storage::read_string(STORAGE_NAMESPACE, STORAGE_WIFI_PASSWD, wifi_passwd, &passwd_length) != ESP_OK)
    strlcpy (wifi_passwd, WIFI_AP_DEFAULT_PASSWD, WIFI_PASSWD_MAX_LENGTH+1);

  // Wifi channel
  int channel = WIFI_AP_DEFAULT_CHANNEL;
  if (storage::read_int32(STORAGE_NAMESPACE, STORAGE_WIFI_CHANNEL, &channel) != ESP_OK)
    channel = WIFI_AP_DEFAULT_CHANNEL; 

  // Connect Wifi
  if (mode == WIFI_MODE_AP)
    wifi::wifi_init_softap(wifi_ssid, wifi_passwd, 8, channel);
  else
    wifi::wifi_init_sta(wifi_ssid, wifi_passwd);
}

LED_Display * start_display()
{
  // Init LED MAtrix
    LED_Display *dis = new LED_Display();
    dis->setTaskName("LEDs");
		dis->setTaskPriority(2);
    dis->setCore(1);
		dis->start();

    return dis;
}

void start_uarts(LED_Display *dis)
{
  asio::io_context io_context;
  uart_server *servers[3];

  for (int i = 0; i < 3 ; i++)
  {
    // Aux vars
    char STORAGE_KEY[50];

    // ENABLE
    int32_t enabled = 0;
    sprintf(STORAGE_KEY, STORAGE_UART_ENABLE, i);
    if (storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &enabled) != ESP_OK)
      enabled = UART_DEFAULT_ENABLE[i];
    
    if (enabled == 0)
      continue;

    // Bauds
    int32_t bauds = 0;
    sprintf(STORAGE_KEY, STORAGE_UART_BAUDS, i);
    if (storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &bauds) != ESP_OK)
      bauds = UART_DEFAULT_BAUDS;

    // TCP Port
    int32_t tcp_port = 0;
    sprintf(STORAGE_KEY, STORAGE_UART_TCP_PORT, i);
    if (storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &tcp_port) != ESP_OK)
      tcp_port = UART_DEFAULT_TCP_PORT[i];

    // TX Pin
    int32_t tx_pin = 0;
    sprintf(STORAGE_KEY, STORAGE_UART_TX_PIN, i);
    if (storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &tx_pin) != ESP_OK)
      tx_pin = UART_DEFAULT_TX_PIN[i];

    // RX Pin
    int32_t rx_pin = 0;
    sprintf(STORAGE_KEY, STORAGE_UART_RX_PIN, i);
    if (storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &rx_pin) != ESP_OK)
      rx_pin = UART_DEFAULT_RX_PIN[i];

    // TX Buffer
    int32_t tx_buffer = 0;
    sprintf(STORAGE_KEY, STORAGE_UART_TX_BUFFER, i);
    if (storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &tx_buffer) != ESP_OK)
      tx_buffer = UART_DEFAULT_BUFFER;

    // RX Buffer
    int32_t rx_buffer = 0;
    sprintf(STORAGE_KEY, STORAGE_UART_RX_BUFFER, i);
    if (storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &rx_buffer) != ESP_OK)
      rx_buffer = UART_DEFAULT_BUFFER;

    // Data bits
    int32_t data_bits = 0;
    sprintf(STORAGE_KEY, STORAGE_UART_DATA_BITS, i);
    if (storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &data_bits) != ESP_OK)
      data_bits = UART_DEFAULT_DATA_BITS;

    // Parity
    int32_t parity = 0;
    sprintf(STORAGE_KEY, STORAGE_UART_PARITY, i);
    if (storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &parity) != ESP_OK)
      parity = UART_DEFAULT_PARITY;

    // Stop bits
    int32_t stop_bits = 0;
    sprintf(STORAGE_KEY, STORAGE_UART_STOP_BITS, i);
    if (storage::read_int32(STORAGE_NAMESPACE, STORAGE_KEY, &stop_bits) != ESP_OK)
      stop_bits = UART_DEFAULT_STOP_BITS;

    ESP_LOGI("START_UART", "Uart N: %d, Enabled: %d, Bauds: %d, TCP: %d, TXPin: %d, RXPin: %d, TXBuff: %d, RXBuff: %d, DataBits: %d, Parity: %d, StopBits: %d", i, enabled, bauds, tcp_port, tx_pin, rx_pin, tx_buffer, rx_buffer, data_bits, parity, stop_bits);
    configure_uart((uart_port_t)i, bauds, tx_pin, rx_pin, tx_buffer, rx_buffer, data_bits, (uart_parity_t)parity, stop_bits);
    ESP_LOGI("START_UART", "Server Uart N: %d, RXLed: %d, TXLed: %d, ConnLed: %d", i, LED_DEFAULT_RX[i], LED_DEFAULT_TX[i], LED_DEFAULT_CONNECTED[i]);
    servers[i] = new uart_server(&io_context, tcp_port, (uart_port_t)i, dis, LED_DEFAULT_RX[i], LED_DEFAULT_TX[i], LED_DEFAULT_CONNECTED[i]);
  }

  // Block here forever
  io_context.run();

  ESP_LOGI("start_uart", "exit");

}

extern "C" void app_main()
{
    // Disable logging
    //esp_log_level_set("*", ESP_LOG_NONE);

    //Initialize NVS
    initNVS();

    // Wait for character to enter console mode
    int use_console = wait_for_console_mode();
    if (use_console)
    {
        // Remove old driver
        uart_driver_delete(UART_NUM_0);
        start_and_run_console();
    }
    // Remove old driver
    uart_driver_delete(UART_NUM_0);

    ESP_LOGI("MAIN", "Init");    

    // Start diaplay
    LED_Display *dis = start_display();
    ::vTaskDelay(100/portTICK_PERIOD_MS);
    // Start Wifi
    start_wifi(dis);
    // Start Uarts
    start_uarts(dis);
    
    ESP_LOGI("MAIN", "Exit");
}