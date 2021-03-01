#include "I2SSampler.h"

struct I2SSampler i2sSamplerStruct;


i2s_port_t getI2SPort()
{
    return i2sSamplerStruct.i2sPort;
}



int32_t getBufferSizeInBytes(void)
{
    return i2sSamplerStruct.bufferSizeInBytes;
}


int16_t *getCapturedAudioBuffer(void)
{
    return i2sSamplerStruct.pCapturedAudioBuffer;
}


void addSample(uint16_t sample)
{
     int16_t *p_tempBuffAddress = NULL;
     /*Add the Sample to current Audio Buffer*/
     i2sSamplerStruct.pCurrentAudioBuffer[i2sSamplerStruct.audioBufferPosition] = sample;
     i2sSamplerStruct.audioBufferPosition++;
    /*Have we filled the buffer with data*/
    if( i2sSamplerStruct.audioBufferPosition == i2sSamplerStruct.bufferSizeInSamples)
    {
        /* Swap to the other buffer */
         p_tempBuffAddress = i2sSamplerStruct.pCurrentAudioBuffer;
         i2sSamplerStruct.pCurrentAudioBuffer = i2sSamplerStruct.pCurrentAudioBuffer;
         i2sSamplerStruct.pCapturedAudioBuffer = p_tempBuffAddress;

         /*reset the buffer position*/
         i2sSamplerStruct.audioBufferPosition = 0;

         /*Tell the Writer Task to Save the Data*/
         xTaskNotify(i2sSamplerStruct.writerTaskHandle, 1, eIncrement);
    }
}

 
void i2sReaderTask( void *param )
{
    struct I2SSampler *sampler = (struct I2SSampler*)param;
    while(1)
    {
        /* wait for some data to arrive on the queue */
        i2s_event_t evt;
        if( xQueueReceive( sampler->i2sQueue, &evt, portMAX_DELAY) == pdPASS )
        {
            if(evt.type == I2S_EVENT_RX_DONE)
            {
                size_t bytesRead = 0;
                do
                {
                    /*Read Data from the I2S Peripheral*/
                    uint8_t i2sData[1024];
                    /*Read from I2S*/
                    i2s_read(getI2SPort(), i2sData, 1024, &bytesRead, 10);
                    /*Process the Raw Data*/
                    processI2SData(i2sData, bytesRead);
                } while (bytesRead > 0); 
            }
           i2sSamplerStruct.collectedBuffCount++;
           if( i2sSamplerStruct.collectedBuffCount > 3)
           {
               i2s_stop(getI2SPort());
               i2sSamplerStruct.collectedBuffCount = 0;
           }
        }
    }
}


void start(i2s_port_t i2sPort, i2s_config_t *i2sConfig, int32_t bufferSizeInBytes, TaskHandle_t writerTaskHandle) 
{
    i2sSamplerStruct.i2sPort = i2sPort;
    i2sSamplerStruct.writerTaskHandle = writerTaskHandle;
    i2sSamplerStruct.bufferSizeInSamples = bufferSizeInBytes / sizeof(int16_t);
    i2sSamplerStruct.bufferSizeInBytes = bufferSizeInBytes;
    i2sSamplerStruct.pAudioBuffer1 = (int16_t*)malloc(bufferSizeInBytes);
    i2sSamplerStruct.pAudioBuffer2 = (int16_t*)malloc(bufferSizeInBytes);

    i2sSamplerStruct.pCurrentAudioBuffer = i2sSamplerStruct.pAudioBuffer1;
    i2sSamplerStruct.pCapturedAudioBuffer = i2sSamplerStruct.pAudioBuffer2;

    i2sSamplerStruct.writerTaskHandle = writerTaskHandle;

    /*Install and start the i2s driver*/
    i2s_driver_install(i2sSamplerStruct.i2sPort, i2sConfig, 4, &i2sSamplerStruct.i2sQueue);

    /*Set up the I2S configuration from the subclass*/
    configureI2S();

    /*Start Reader Task*/
    xTaskCreatePinnedToCore(i2sReaderTask, "i2s Reader Task", 4096, &i2sSamplerStruct, 1, &i2sSamplerStruct.readerTaskHandle, 0);
}


void I2SMEMSSampler(i2s_pin_config_t i2sPins, bool fixSPH0645)
{
    i2sSamplerStruct.i2sPins = i2sPins;
    i2sSamplerStruct.fixSPH0645 = fixSPH0645;
}


void configureI2S(void)
{
    #if 0
    if( i2sSamplerStruct.fixSPH0645 )
    {
        REG_SET_BIT(I2S_TIMING_REG(getI2SPort()), REG(9));
        REG_SET_BIT(I2S_CONF_REG(getI2SPort()), I2S_RX_MSB_SHIFT);
    }
    #endif

    i2s_set_pin(getI2SPort(), &i2sSamplerStruct.i2sPins);
    i2s_stop(getI2SPort());
    i2sSamplerStruct.is_i2s_start = false;
}



void processI2SData(uint8_t * p_i2sData, size_t bytesRead)
{
    int32_t *samples = (int32_t*)p_i2sData;
    for ( int i = 0; i < bytesRead/4; i++ )
    {
        /*You may need to vary the >>11 to fit your volume - ideally we'd
        have some kind of AGC here */
        addSample(samples[i] >> 11);
    }
}
