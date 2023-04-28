#include "DCP_Sensor.h"
#include "LightSwitch.h"

#ifdef CFG_PLF_RV32
#define duet_adc_init asr_adc_init
#define duet_adc_get asr_adc_get
#endif

void DCP_Sensor::Init(void)
{
    iVoltage = 0;

    adc_config_struct.port = ADC_CHANNEL_NUM4;
    adc_config_struct.config.sampling_cycle = 10;
    adc_config_struct.priv = NULL;
    duet_adc_init(&adc_config_struct);
}

int32_t DCP_Sensor::Get(void)
{
    int32_t dataBuf[10];
    int32_t iSum = 0;
    uint8_t i = 0, ucCnt = adc_config_struct.config.sampling_cycle;

    while (ucCnt--)
    {
        dataBuf[i] = duet_adc_get(&adc_config_struct);
        iSum += dataBuf[i];
        i++;
    }
    return (iSum / 10);
}

extern LightSwitch kLightSwitch1;
void DCP_Sensor::Report(int32_t iVol)
{
    uint16_t sBrightness;
    sBrightness = (uint8_t)(MAX_BRIGHTNESS * ((float)iVol / MAX_VOLTAGE));
    kLightSwitch1.SwitchChangeBrightness(sBrightness);
}