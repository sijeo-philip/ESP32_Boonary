#include <stdio.h>
#include "esp_spiffs.h"
#include "esp_log.h"

#include <stdlib.h>
#include <dirent.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "I2SSampler.h"
#include "common_vars.h"
#include "connect.h"
#include "fetch.h"

#define TAG_CONNECTION "CONNECTION"


/* NEED TO DO better Handling of the Semaphore */
xSemaphoreHandle onConnectionHandler;
TimerHandle_t samplingTimerHandle;

/* i2s configuration for reading the both channels of I2S */
i2s_config_t i2sMemsConfigBothChannel = {
.mode = (I2S_MODE_MASTER | I2S_MODE_RX),
.sample_rate = 16000,
.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
.communication_format = I2S_COMM_FORMAT_I2S,
.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, 
.dma_buf_count = 4,
.dma_buf_len = 1024,
.use_apll = false,
.tx_desc_auto_clear = false,
.fixed_mclk = 0 
};

/*i2s Pins Configuration */
i2s_pin_config_t i2sPins = {
.bck_io_num = GPIO_NUM_26, 
.ws_io_num = GPIO_NUM_25,
.data_out_num = I2S_PIN_NO_CHANGE,
.data_in_num = GPIO_NUM_33
};



/*Task to Write the collected data to End Node or SPIFF*/
void i2sMemsWriterTask(void *param)
{
  #if 0
  struct I2SSampler *sampler = (struct I2SSampler*)param;
  #endif 


  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);
  while(1)
  {
    /*Wait for some samples to save */
    uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
    if( ulNotificationValue > 0 )
    {
      ESP_ERROR_CHECK(esp_wifi_start());
      xSemaphoreTake(onConnectionHandler, portMAX_DELAY);
      ESP_LOGI(TAG_CONNECTION, "Processing...");
      struct DataParams dataParams;
      dataParams.onGotData = NULL;
      dataParams.body = (char*)getCapturedAudioBuffer();
      dataParams.headerCount = 1;
      dataParams.method = Post;
      Header headerContentType = {
          .key = "content-type",
          .val = "application/octet-stream"
      };

      dataParams.header[0] = headerContentType;
      /*Need to change this URL as per the Set server IP value */
      data_transact("http://192.168.1.4/audio_sample", &dataParams);
      if (dataParams.status == 200)
      {
        ESP_LOGI(TAG_CONNECTION, "Data Is Succesfully Sent\n");
      }
      ESP_ERROR_CHECK(esp_wifi_stop());
    }
  }
}


/*Callback for Sampling Timer */
void samplingTimerCallback( TimerHandle_t xTimer)
{
  if (i2sSamplerStruct.is_i2s_start == false)
  {
    i2s_start(getI2SPort());
    i2sSamplerStruct.is_i2s_start = true;
    ESP_LOGI("I2S_PORT", "Started Sampling...\n");
  }
  else
    ESP_LOGI("I2S_PORT", "Already Sampling...\n");

}


void app_main(void)
{
  /*Using INMP MEMS Mic as Sensor */
  I2SMEMSSampler(i2sPins, false);
  onConnectionHandler = xSemaphoreCreateBinary();
  samplingTimerHandle =xTimerCreate("Sample Timer", pdMS_TO_TICKS(5000), pdTRUE, 0, samplingTimerCallback);
  wifiInit();

  TaskHandle_t i2sMemsWriterTaskHandle;
  xTaskCreatePinnedToCore(i2sMemsWriterTask, "I2S Mems Writer", 4096, &i2sSamplerStruct, 1, &i2sMemsWriterTaskHandle, 1);

  start(I2S_NUM_1, &i2sMemsConfigBothChannel, 32768, i2sMemsWriterTaskHandle);
  /*TODO:
   * 1. Need to set up interval for sampling the MIC
   * 2. Use a Timer based system.
   * 3. Use I2S_start and i2s_stop to control the sampling time
   * **/
  while(1)
  {

  }

}
