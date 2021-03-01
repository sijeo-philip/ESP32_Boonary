#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define SSID CONFIG_WIFI_SSID
#define PASSWORD CONFIG_WIFI_PASSWORD


xSemaphoreHandle onConnectionHandler;


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
  switch (event->event_id)
  {
  case SYSTEM_EVENT_STA_START:
    printf("Connecting... \n");
    esp_wifi_connect();
    break;
  case SYSTEM_EVENT_STA_CONNECTED:
    printf("Connected...\n");
    /* code */
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
      printf("Got IP... \n");
      xSemaphoreGive(onConnectionHandler);
  break;

  case SYSTEM_EVENT_STA_DISCONNECTED:
    printf("Disconnected...\n");
  break;
  
  default:
    break;
  }
  return ESP_OK;
}

void wifiInit(void)
{
  ESP_ERROR_CHECK(nvs_flash_init());
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
  /* FOR V4.1 IDF
    esp_netif_init(); to be used instead of tcpip_adapter_init();
    esp_event_loop_create_default(); instead of esp_event_loop_init();
    call esp_netif_create_default_sta();
    */
   
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

void onConnected (void * params)
{
  while(1)
  {
    if(xSemaphoreTake(onConnectionHandler, 10*1000/portTICK_PERIOD_MS))
    {
      /* Do Something */
        xSemaphoreTake(onConnectionHandler, portMAX_DELAY);
    }
    else
    {
      ESP_LOGE("Connection","Could not Connect");
      esp_restart();
    }
  }
}


void app_main(void)
{
  onConnectionHandler =xSemaphoreCreateBinary();
  wifiInit();
  xTaskCreate(&onConnected, "On Connection", 1024 * 4, NULL, 5, NULL);
}
