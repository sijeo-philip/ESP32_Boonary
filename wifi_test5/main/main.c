#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "fetch.h"
#include "connect.h"
#include "cJSON.h"

#define TAG "DATA"

xSemaphoreHandle onConnectionHandler;

void onGotData(char *incomingBuffer, char* output)
{
    cJSON *payload = cJSON_Parse(incomingBuffer);
    cJSON *contents = cJSON_GetObjectItem(payload, "contents");
    cJSON *quotes = cJSON_GetObjectItem(contents, "quotes");
    cJSON *quotesElement;
    cJSON_ArrayForEach(quotesElement, quotes)
    {
        cJSON *quote = cJSON_GetObjectItem(quotesElement, "quote");
        ESP_LOGI(TAG,"%s", quote->valuestring);
        strcpy(output, quote->valuestring);
    }
    cJSON_Delete(payload);
}

void createBody(char *number, char * message, char *out)
{
sprintf(out, 
"{"
" \"message\": ["
"       {"
"       "
"           \"content\": \"%s\", "
"           \"destination_number\": \"%s\","
"           \"format\": \"SMS\" "
"       }"
"    ]"
"}", message, number);
}

void onConnected ( void * params )
{
  while(1)
  {
    if(xSemaphoreTake(onConnectionHandler, 10*1000/portTICK_RATE_MS))
    {
    
      ESP_LOGI(TAG, "Processing");
      struct FetchParams fetchParams;
      fetchParams.onGotData = onGotData;
      fetchParams.body = NULL;
      fetchParams.headerCount = 0;
      fetchParams.method = Get;

      fetch("http://quotes.rest/qod", &fetchParams);
      if(fetchParams.status == 200)
      {
        //send sms
        struct FetchParams smsStruct;
        smsStruct.onGotData = NULL;
        smsStruct.method = Post;

        Header headerContentType = {
          .key = "Content-Type",
          .val = "application/json"
        };

        Header headerAccept= {
          .key = "Accept",
          .val = "application/json"
        };

        Header headerAuthorization = {
          .key = "Authorization",
          .val = "Basic a3NScUxxOUZCeU9bmVHV"
        };
        smsStruct.header[0] = headerAuthorization;
        smsStruct.header[1] = headerAccept;
        smsStruct.header[2] = headerContentType;
        smsStruct.headerCount = 3;
        char buffer[1024];

        createBody("+919167447337", fetchParams.message, buffer);
        smsStruct.body = buffer;
        fetch("https://api.messagemedia.com/v1/messages", &smsStruct);
      }
      ESP_LOGI(TAG, "%s", fetchParams.message);
      ESP_LOGI(TAG, "Done!");
      esp_wifi_disconnect();
      xSemaphoreTake(onConnectionHandler, portMAX_DELAY);
    }
    else
    {
      ESP_LOGE(TAG, "Failed to Connect. Retry In");
      for(int i = 0; i < 5; i++)
      {
        ESP_LOGE(TAG, "...%d", i);
        vTaskDelay(1000/portTICK_RATE_MS);
      }
      esp_restart();
    }
  }
}

void app_main(void)
{
  esp_log_level_set(TAG, ESP_LOG_DEBUG);
  onConnectionHandler = xSemaphoreCreateBinary();
  wifiInit();
  xTaskCreate(&onConnected, "On Connection", 1024*4, NULL, 5, NULL);
  
}
