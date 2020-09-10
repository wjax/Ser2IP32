#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "esp_system.h"

class storage
{
    public:
        static esp_err_t read_int32(const char* storage_name, const char *variable_name, int32_t *out_value);
        static esp_err_t write_int32(const char* storage_name, const char *variable_name, int32_t value);
        static esp_err_t read_string(const char* storage_name, const char *variable_name, char* out_string, size_t *max_length);
        static esp_err_t write_string(const char* storage_name, const char *variable_name, const char* value);

        static void format_nvs();
         
};

#endif