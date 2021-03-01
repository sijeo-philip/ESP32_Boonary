#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_http_client.h"
#include "esp_log.h"
#include "cJSON.h"
#include "fetch.h"

#define TAG "HTTPCLIENT"

char* buffer = NULL;
int indexBuff = 0;

void onGotData(char *incomingBuffer);

esp_err_t clientEventHandler(esp_http_client_event_t *evt)
{
    struct FetchParams *fetchParams = (struct FetchParams*) evt->user_data;
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA Len=%d", evt->data_len);
        /* code */
        printf("%.*s\n", evt->data_len, (char*)evt->data);
        if( buffer == NULL )
        {
            buffer = (char *) malloc(evt->data_len);
        }
        else
        {
            buffer = (char *)realloc(buffer, evt->data_len + indexBuff);
        }
        memcpy(&buffer[indexBuff], evt->data, evt->data_len);
        indexBuff += evt->data_len;
        break;

        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            buffer = (char*)realloc(buffer, indexBuff+1);
            memcpy(&buffer[indexBuff], "\0", 1);
            if( fetchParams->onGotData != NULL)
            {
                 fetchParams->onGotData(buffer, fetchParams->message);
            }
            free(buffer);
            buffer = NULL;
            indexBuff = 0;
        break;
    
    default:
        break;
    }
    return ESP_OK;
}


void fetch(char* url, struct FetchParams *fetchParams)
{
    esp_http_client_config_t clientConfig = {
        .url = url,
        .event_handler = clientEventHandler, 
        .user_data = fetchParams
    };
    esp_http_client_handle_t client = esp_http_client_init(&clientConfig);
    if( fetchParams->method ==  Post)
    {
        esp_http_client_set_method(client, HTTP_METHOD_POST);
    }
    for( int i = 0; i < fetchParams->headerCount; i++ )
    {
        esp_http_client_set_header(client, fetchParams->header[i].key, fetchParams->header[i].val);
    }
    if(fetchParams->body != NULL)
    {
        esp_http_client_set_post_field(client, fetchParams->body, strlen(fetchParams->body));
    }
    esp_err_t err = esp_http_client_perform(client);
    fetchParams->status = esp_http_client_get_status_code(client);
    if(err == ESP_OK)
    {
        ESP_LOGI(TAG, "HTTP GET STATUS = %d", esp_http_client_get_status_code(client));
    }
    else
    {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}