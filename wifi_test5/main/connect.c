#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "fetch.h"


#define SSID CONFIG_WIFI_SSID
#define PASSWORD CONFIG_WIFI_PASSWORD

char *TAG = "CONNECTION";

extern xSemaphoreHandle onConnectionHandler;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
  switch (event->event_id)
  {
  case SYSTEM_EVENT_STA_START:
   esp_wifi_connect();
   ESP_LOGI(TAG, "Connecting...\n");
  break;
  
  case SYSTEM_EVENT_STA_CONNECTED:
    ESP_LOGI(TAG, "Connected...\n");
  break;

  case SYSTEM_EVENT_STA_GOT_IP:
    ESP_LOGI(TAG, "GOT IP...\n");
    xSemaphoreGive(onConnectionHandler);
  break;

  case SYSTEM_EVENT_STA_DISCONNECTED:
     ESP_LOGI(TAG, "Disconnected ...\n");
  break;
   
  default:
    break;
  }
  return ESP_OK;
}

void wifiInit( void )
{
  ESP_ERROR_CHECK(nvs_flash_init());
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

  wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

  wifi_config_t wifi_config = {
    .sta = {
      .ssid = SSID,
      .password = PASSWORD
    }
  };
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}
