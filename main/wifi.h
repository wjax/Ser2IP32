#ifndef _WIFI_H_
#define _WIFI_H_

#include "esp_wifi.h"


class wifi
{
    private:
     static void wifi_event_handler_softAP(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
     static void wifi_event_handler_station(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

    public:
     static void wifi_init_softap(const char * ssid, const char * password, uint8_t max_connections, uint8_t channel);
     static void wifi_init_sta(const char * ssid, const char * password);
};

#endif