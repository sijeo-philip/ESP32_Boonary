#ifndef _I2SSAMPLER_H_
#define _I2SSAMPLER_H_


#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_wifi.h"
#include "driver/i2s.h"

struct I2SSampler
{
    /*Double the Buffer so that we can Capture data while sending Data */
    int16_t *pAudioBuffer1;
    int16_t *pAudioBuffer2;

    /*Current Position of Audio Buffer*/
    int32_t audioBufferPosition;
    /*Current Audio Buffer */
    int16_t *pCurrentAudioBuffer;
    /*Buffer Containing Samples that have been captured already*/
    int16_t *pCapturedAudioBuffer;
    /*Size of Audio Buffers in bytes*/
    int32_t  bufferSizeInBytes;
    /*Size of Audio Buffer in Samples */
    int32_t bufferSizeInSamples;
    /*I2S reader Task*/
    TaskHandle_t readerTaskHandle;
    /*Writer Task*/
    TaskHandle_t writerTaskHandle;
    /*i2s reader Queue */
    QueueHandle_t i2sQueue;
    /*i2s Port*/
    i2s_port_t i2sPort;
    /*Pin Configuration of I2S pins*/
    i2s_pin_config_t i2sPins;
    /*bool for start / stop of i2s peripheral True if start*/
    bool is_i2s_start;
    /*Collected Buffer Count*/
    int32_t collectedBuffCount;
    /*bool for sensor INMP or SPH0645*/
    bool fixSPH0645;
};


void addSample(uint16_t sample);
void configureI2S(void);            /* Code in I2SMEMSSampler.cpp*/
void processI2SData(uint8_t * p_i2sData, size_t bytesRead);  /*Code in I2SMEMSSampler.cpp*/
i2s_port_t getI2SPort(void);
int32_t getBufferSizeInBytes(void);
int16_t *getCapturedAudioBuffer(void);


void start(i2s_port_t i2sPort, i2s_config_t *i2sConfig, int32_t bufferSizeInBytes, TaskHandle_t writerTaskHandle);

/*Reader Task for Reading I2S samples from the MIC*/
void i2sReaderTask(void *params);


void I2SMEMSSampler(i2s_pin_config_t i2sPins, bool fixSPH0645);

#endif