/*
 * ICP20100.h
 *
 *  Created on: Feb 16, 2025
 *      Author: yunus
 */

 #ifndef INC_ICP20100_H_
 #define INC_ICP20100_H_
 
 
 #include <stdint.h>
 
 typedef int8_t (*ICP20100_Read_t)(void* intf, uint8_t reg, uint8_t *pRxData, uint8_t len);
 typedef int8_t (*ICP20100_Write_t)(void* intf, uint8_t reg, uint8_t *pTxData, uint8_t len);
 typedef void   (*ICP20100_Delay_t)(uint32_t ms);
 
 
 typedef enum{
     MODE0 = 0,
     MODE1 = 1,
     MODE2 = 2,
     MODE3 = 3,
     MODE4 = 4,
 }ICP20100_MEAS_CONFIG;
 
 
 typedef enum{
     STANDBY = 0,
     TRIGGER = 1
 }ICP20100_FORCED_MEAS_TRIGGER;
 
 
 typedef enum{
     TRIGGER_FORCED = 0,
     DUTY_CYCLED = 1
 }ICP20100_MEAS_MODE;
 
 
 typedef enum{
     NORMAL = 0,
     ACTİVE = 1
 }ICP20100_POWER_MODE;
 
 
 
 typedef enum{
     PRESSURE_FIRST = 0,
     TEMPERATURE_ONLY = 1,
     TEMPERATURE_FIRST = 2,
     PRESSURE_ONLY = 3
 }ICP20100_FIFO_READOUT_MODE;
 
 
 
 typedef struct ICP20100_Settings{
     ICP20100_MEAS_CONFIG meas_config;
     ICP20100_FORCED_MEAS_TRIGGER forced_meas;
     ICP20100_MEAS_MODE meas_mode;
     ICP20100_POWER_MODE power_mode;
     ICP20100_FIFO_READOUT_MODE fifo_read;
 }ICP20100_Settings;
 
 
 
 typedef struct ICP20100_Device_s{
     uint8_t chip_id;
     void* intf;
     ICP20100_Read_t read;
     ICP20100_Write_t write;
     ICP20100_Delay_t delay;
     ICP20100_Settings settings;
 }ICP20100_Device_t;
 
 
 
 int8_t ICP20100_ReadRegister(ICP20100_Device_t* dev, uint8_t reg, uint8_t *pRxBuffer);
 int8_t ICP20100_WriteRegister(ICP20100_Device_t* dev, uint8_t reg, uint8_t *pTxBuffer);
 int8_t ICP20100_BootSequence(ICP20100_Device_t* dev);
 void ICP20100_SetMeasConfig(ICP20100_Device_t* dev, ICP20100_MEAS_CONFIG range);
 void ICP20100_SetForcedMeasTrigger(ICP20100_Device_t* dev, ICP20100_FORCED_MEAS_TRIGGER range);
 void ICP20100_SetMeasMode(ICP20100_Device_t* dev, ICP20100_MEAS_MODE range);
 void ICP20100_SetPowerMode(ICP20100_Device_t* dev, ICP20100_POWER_MODE range);
 void ICP20100_SetFIFOReadoutMode(ICP20100_Device_t* dev, ICP20100_FIFO_READOUT_MODE range);
 int ICP20100_ProcessRawData(ICP20100_Device_t* dev, uint8_t packet_cnt, uint8_t *data, int32_t *pressure, int32_t *temperature);
 void ICP20100_ReadFifo(ICP20100_Device_t* dev, uint8_t address, uint8_t *data, uint8_t lenght);
 void ICP20100_ReadData(ICP20100_Device_t* dev);
 
 
 
 
 
 
 #endif /* INC_ICP20100_H_ */
 