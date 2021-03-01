#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_http_client.h"
#include "esp_log.h"
#include "fetch.h"

#define HTTP_TAG "HTTPCLIENT"

char* buffer = NULL;
int indexBuff = 0;

void onGotData(char *incomingBuffer);

esp_err_t clientEventHandler( esp_http_client_event_t *evt)
{
    struct DataParams *dataParams = (struct DataParams*)evt->user_data; 
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(HTTP_TAG, "Connected to HTTP Server\n...");
        /*Send Data to the Server */
        break;
    case HTTP_EVENT_HEADERS_SENT:
        ESP_LOGI(HTTP_TAG, "%d Headers Sent\n", dataParams->headerCount);
        break;

    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(HTTP_TAG, "HTTP EVENT ON FINISH");
        ESP_LOGI(HTTP_TAG, "Status of Transaction %d\n", dataParams->status);
        break;
        
    default:
        break;
    }
    return ESP_OK;
}


void data_transact (char* url, struct DataParams *dataParams)
{
    esp_http_client_config_t clientConfig = {
        .url = url,
        .event_handler = clientEventHandler,
        .user_data = dataParams
    };
    esp_http_client_handle_t client = esp_http_client_init( &clientConfig );
    if ( dataParams->method == Post )
    {
        esp_http_client_set_method(client, HTTP_METHOD_POST);
    }
    for ( int i=0; i < dataParams->headerCount; i++ )
    {
        esp_http_client_set_header(client, dataParams->header[i].key, dataParams->header[i].val);
    }
    if( dataParams->body != NULL )
    {
        esp_http_client_set_post_field(client, dataParams->body, strlen(dataParams->body));
    }
    esp_err_t err = esp_http_client_perform(client);
    dataParams->status = esp_http_client_get_status_code(client);
    if( err == ESP_OK )
    {
        ESP_LOGI(HTTP_TAG, "HTTP GET STATUS= %d", dataParams->status);
    }
    else 
    {
        ESP_LOGE(HTTP_TAG, "HTTP Request Failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}
