#include "storage.h"
#include "nvs_flash.h"
#include "nvs.h"


esp_err_t storage::read_int32 (const char* storage_name, const char *variable_name, int32_t *out_value)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open
    err = nvs_open(storage_name, NVS_READONLY, &my_handle);
    if (err != ESP_OK) return err;

    // Read
    err = nvs_get_i32(my_handle, variable_name, out_value);

    // Close
    nvs_close(my_handle);
    return err;
}

esp_err_t storage::write_int32 (const char* storage_name, const char *variable_name, int32_t value)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open
    err = nvs_open(storage_name, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    // Write
    err = nvs_set_i32(my_handle, variable_name, value);
    if (err != ESP_OK)
        err = nvs_commit(my_handle);

    // Commit written value.
    // After setting any values, nvs_commit() must be called to ensure changes are written
    // to flash storage. Implementations may write to storage at other times,
    // but this is not guaranteed.

    // Close
    nvs_close(my_handle);
    return err;
}

esp_err_t storage::read_string (const char* storage_name, const char *variable_name, char* out_string, size_t *length)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open
    err = nvs_open(storage_name, NVS_READONLY, &my_handle);
    if (err != ESP_OK) return err;

    size_t t_length = 0;
    if ( (err = nvs_get_str(my_handle, variable_name, NULL, &t_length)) == ESP_OK) 
    {
        if (*length >= t_length)
            err = nvs_get_str(my_handle, variable_name, out_string, length);
    }

    // Close
    nvs_close(my_handle);
    return err;
}

esp_err_t storage::write_string(const char* storage_name, const char *variable_name, const char* value)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open
    err = nvs_open(storage_name, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    // Write
    err = nvs_set_str(my_handle, variable_name, value);
    if (err != ESP_OK) 
        err = nvs_commit(my_handle);

    // Commit written value.
    // After setting any values, nvs_commit() must be called to ensure changes are written
    // to flash storage. Implementations may write to storage at other times,
    // but this is not guaranteed.

    // Close
    nvs_close(my_handle);
    return err;
}

void storage::format_nvs()
{
    // Clear NVS
    nvs_flash_erase();
    nvs_flash_init();
}