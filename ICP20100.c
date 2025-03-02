/*
 * ICP20100.c
 *
 *  Created on: Feb 16, 2025
 *      Author: yunus
 */


 #include "ICP20100.h"
 #include "main.h"
 #include <stdint.h>
 
 
 #define TRIM1_MSB 						0X05
 #define TRIM2_LSB 						0X06
 #define TRIM2_MSB 						0X07
 
 #define DEVICE_ID 						0X0C
 
 #define IO_DRIVE_STRENGHT 				0X0D
 
 #define OTP_CONFIG1 					0XAC
 #define OTP_MR_LSB 						0XAD
 #define OTP_MR_MSB 						0XAE
 #define OTP_MRA_LSB 					0XAF
 #define OTP_MRA_MSB 					0XB0
 #define OTP_MRB_LSB 					0XB1
 #define OTP_MRB_MSB 					0XB2
 #define OTP_ADDRESS_REG 				0XB5
 #define OTP_COMMAND_REG 				0XB6
 #define OTP_RDATA 						0XB8
 #define OTP_STATUS 						0XB9
 #define OTP_DBG2 						0XBC
 #define OTP_STATUS2 					0XBF
 
 #define MASTER_LOCK 					0XBE
 
 #define MODE_SELECT 					0XC0
 #define DEVICE_STATUS 					0XCD
 #define I3C_INFO 						0XCE
 #define VERSION 						0XD3
 #define SPI_MODE 						0XC5
 
 #define INTERRUPT_STATUS 				0XC1
 #define INTERRUPT_MASK 					0XC2
 
 #define FIFO_CONFIG 					0XC3
 #define FIFO_FILL 						0XC4
 
 #define PRESS_ABS_LSB 					0XC7
 #define PRESS_ABS_MSB 					0XC8
 #define PRESS_DELTA_LSB 				0XC9
 #define PRESS_DELTA_MSB 				0XCA
 
 #define PRESS_DATA_0 					0XFA
 #define PRESS_DATA_1 					0XFB
 #define PRESS_DATA_2 					0XFC
 
 #define TEMP_DATA_0 					0XFD
 #define TEMP_DATA_1 					0XFE
 #define TEMP_DATA_2 					0XFF
 
 #define VERSION_A 						0x00
 #define TEMP_CONVERT_CONST				0.00024795532f
 #define TEMP_CONVERT_C(temp_raw) 		(temp_raw*TEMP_CONVERT_CONST) + 25.0
 
 #define PRESS_CONVERT_CONST				0.3051757813f
 #define PRESS_CONVERT_PA(press_raw)		(press_raw*PRESS_CONVERT_CONST) + 70000.0
 
 
 
 int8_t ICP20100_ReadRegister(ICP20100_Device_t* dev, uint8_t reg, uint8_t *pRxBuffer, uint8_t lenght){
 
     int8_t status = dev->read(dev->intf, reg, pRxBuffer, lenght);
     return status;
 }
 
 
 int8_t ICP20100_WriteRegister(ICP20100_Device_t* dev, uint8_t reg, uint8_t *pTxBuffer){
 
     int8_t status = dev->write(dev->intf, reg, pTxBuffer, 1);
     return status;
 }
 
 
 int8_t ICP20100_BootSequence(ICP20100_Device_t* dev){
 
     uint8_t version;
     uint8_t bootup_status;
     uint8_t master_lock;
     uint8_t otp_config1;
     uint8_t otp_dbg2;
     uint8_t otp_mrx;
     uint8_t otp_add;
     uint8_t otp_status;
     uint8_t offset;
     uint8_t gain;
     uint8_t HFosc;
     uint8_t trim1_msb;
     uint8_t rData;
     uint8_t trim2_lsb;
     uint8_t power_mode;
     int	next = 0;
 
     ICP20100_ReadRegister(dev, VERSION, &version);
     ICP20100_ReadRegister(dev, OTP_STATUS2, &bootup_status);
 
 
     if(version == VERSION_A){
         bootup_status &= 0x01;
         if(bootup_status == 0){
             next = 1;
         }
     }
 
     if(next != 1){
         return 0;
     }
 
     if(next == 1){
 
         //regMap.MODE_SELECT.POWER_MODE = 1   Wait 4ms;
         ICP20100_ReadRegister(dev, MODE_SELECT, &power_mode, 1);
         power_mode |= 0b00000100;
         ICP20100_WriteRegister(dev, MODE_SELECT, &power_mode);
         dev->delay_ms(4);
 
 
         //Unlock the main registers
         master_lock = 0x1f;
         ICP20100_WriteRegister(dev, MASTER_LOCK, &master_lock);
 
 
         //Enable the OTP and the write switch
         ICP20100_ReadRegister(dev, OTP_CONFIG1, &otp_config1, 1);
         otp_config1 |= 0b00000011;
         ICP20100_WriteRegister(dev, OTP_CONFIG1, &otp_config1);
         dev->delay_us(10); //wait 10us
 
 
         //Toggle the OTP reset pin
         ICP20100_ReadRegister(dev, OTP_DBG2, &otp_dbg2, 1);
         otp_dbg2 |= 0b10000000;
         ICP20100_WriteRegister(dev, OTP_DBG2, &otp_dbg2);
         dev->delay(10);	//wait 10us
         otp_dbg2 |= 0b00000000;
         ICP20100_WriteRegister(dev, OTP_DBG2, &otp_dbg2);
         dev->delay_us(10);	//wait 10us
 
 
 
         //Program redundant read
         otp_mrx = 0x04;
         ICP20100_WriteRegister(dev, OTP_MRA_LSB, &otp_mrx);
         ICP20100_WriteRegister(dev, OTP_MRA_MSB, &otp_mrx);
         otp_mrx = 0x21;
         ICP20100_WriteRegister(dev, OTP_MRB_LSB, &otp_mrx);
         otp_mrx = 0x20;
         ICP20100_WriteRegister(dev, OTP_MRB_MSB, &otp_mrx);
         otp_mrx = 0x10;
         ICP20100_WriteRegister(dev, OTP_MR_LSB, &otp_mrx);
         otp_mrx = 0x80;
         ICP20100_WriteRegister(dev, OTP_MR_LSB, &otp_mrx);
 
 
 
         //Write the address content and read command
         otp_add = 0xF8;
         ICP20100_WriteRegister(dev, OTP_ADDRESS_REG, &otp_add);  // for offset
         ICP20100_ReadRegister(dev, OTP_COMMAND_REG, &otp_add, 1);
         otp_add &= 0b11110000;
         otp_add |= 0b00010000;
         ICP20100_WriteRegister(dev, OTP_COMMAND_REG, &otp_add);   // read action
 
 
 
         //Wait for the OTP read to finish
         while(1){
 
             ICP20100_ReadRegister(dev, OTP_STATUS, &otp_status, 1);
             otp_status &= 0b00000001;
 
             if(otp_status == 0){
                 break;
             }
             dev->delay_ms(1);
         }
 
 
 
         //Read the data from register
         ICP20100_ReadRegister(dev, OTP_RDATA, &offset, 1);
 
 
 
 
         //Write the next address content and read command
         otp_add = 0xF9;
         ICP20100_WriteRegister(dev, OTP_ADDRESS_REG, &otp_add);  // for gain
         ICP20100_ReadRegister(dev, OTP_COMMAND_REG, &otp_add, 1);
         otp_add &= 0b11110000;
         otp_add |= 0b00010000;
         ICP20100_WriteRegister(dev, OTP_COMMAND_REG, &otp_add);  //read action
 
 
 
         //Wait for the OTP read to finish
         while(1){
 
             ICP20100_ReadRegister(dev, OTP_STATUS, &otp_status, 1);
             otp_status &= 0b00000001;
 
             if(otp_status == 0){
                 break;
             }
             dev->delay_ms(1);
         }
 
 
 
         //Read the data from register
         ICP20100_ReadRegister(dev, OTP_RDATA, &gain, 1);
 
 
 
         //Write the next address content and read command
         otp_add = 0xFA;
         ICP20100_WriteRegister(dev, OTP_ADDRESS_REG, &otp_add);  // for HFosc
         ICP20100_ReadRegister(dev, OTP_COMMAND_REG, &otp_add, 1);
         otp_add &= 0b11110000;
         otp_add |= 0b00010000;
         ICP20100_WriteRegister(dev, OTP_COMMAND_REG, &otp_add);  //read action
 
 
 
         //Wait for the OTP read to finish
         while(1){
 
             ICP20100_ReadRegister(dev, OTP_STATUS, &otp_status, 1);
             otp_status &= 0b00000001;
 
             if(otp_status == 0){
                 break;
             }
             dev->delay_ms(1);
         }
 
 
         // Read the data from register
         ICP20100_ReadRegister(dev, OTP_RDATA, &HFosc, 1);
 
 
 
         //Disable the OTP and the write switch
         ICP20100_ReadRegister(dev, OTP_CONFIG1, &otp_config1, 1);
         otp_config1 &= 0b11111100;
         ICP20100_WriteRegister(dev, OTP_CONFIG1, &otp_config1);
         dev->delay_us(10); //wait 10us
 
 
         //Write the Offset to the main registers
         uint8_t trimmed_offset;
         trimmed_offset = offset & 0b00111111;
         ICP20100_ReadRegister(dev, TRIM1_MSB, &trim1_msb, 1);
         trim1_msb |= trimmed_offset;
         ICP20100_WriteRegister(dev, TRIM1_MSB, &trim1_msb);
 
 
 
         //Write the Gain to the main registers without touching the parameter BG_PTAT_TRIM
         uint8_t trimmed_gain;
         trimmed_gain = gain & 0b00000111;
         trimmed_gain = trimmed_gain << 4;
         ICP20100_ReadRegister(dev, TRIM2_MSB, &rData, 1);
         rData &= 0b10001111;
         rData |= trimmed_gain;
         ICP20100_WriteRegister(dev, TRIM2_MSB, &rData);
 
 
 
         //Write the HFosc trim value to the main registers
         ICP20100_ReadRegister(dev, TRIM2_LSB, &trim2_lsb, 1);
         trim2_lsb &= 0b10000000;
         trim2_lsb |= (HFosc & 0b01111111);
         ICP20100_WriteRegister(dev, TRIM2_LSB, trim2_lsb);
 
 
         //Lock the main registers
         uint8_t lock = 0x00;
         ICP20100_WriteRegister(dev, MASTER_LOCK, &lock);
 
 
         // Move to standby
         ICP20100_ReadRegister(dev, MODE_SELECT, &power_mode, 1);
         power_mode &= 0b11111011;
         ICP20100_WriteRegister(dev, MODE_SELECT, &power_mode);
 
 
 
         //Write bootup config status to 1 to avoid re initialization with out power cycle
         ICP20100_ReadRegister(dev, OTP_STATUS2, &bootup_status, 1);
         bootup_status |= 0b00000001;
         ICP20100_WriteRegister(dev, OTP_STATUS2, bootup_status);
 
 
 
         return 1;
     }
 
 }
 
 
 
 void ICP20100_SetMeasConfig(ICP20100_Device_t* dev, ICP20100_MEAS_CONFIG range){
 
     uint8_t modeReg;
     ICP20100_ReadRegister(dev, MODE_SELECT, &modeReg, 1);
     modeReg &= 0b00011111;
     modeReg |=  (range <<5);
     dev->settings.meas_config = range;
     ICP20100_WriteRegister(dev, MODE_SELECT, &modeReg);
 
 }
 
 
 
 void ICP20100_SetForcedMeasTrigger(ICP20100_Device_t* dev, ICP20100_FORCED_MEAS_TRIGGER range){
 
     uint8_t modeReg;
     ICP20100_ReadRegister(dev, MODE_SELECT, &modeReg, 1);
     modeReg &= 0b11101111;
     modeReg |=  (range <<4);
     dev->settings.forced_meas = range;
     ICP20100_WriteRegister(dev, MODE_SELECT, &modeReg);
 }
 
 
 
 void ICP20100_SetMeasMode(ICP20100_Device_t* dev, ICP20100_MEAS_MODE range){
 
     uint8_t modeReg;
     ICP20100_ReadRegister(dev, MODE_SELECT, &modeReg, 1);
     modeReg &= 0b11110111;
     modeReg |=  (range <<3);
     dev->settings.meas_mode = range;
     ICP20100_WriteRegister(dev, MODE_SELECT, &modeReg);
 }
 
 
 
 void ICP20100_SetPowerMode(ICP20100_Device_t* dev, ICP20100_POWER_MODE range){
 
     uint8_t modeReg;
     ICP20100_ReadRegister(dev, MODE_SELECT, &modeReg, 1);
     modeReg &= 0b11111011;
     modeReg |=  (range <<2);
     dev->settings.power_mode = range;
     ICP20100_WriteRegister(dev, MODE_SELECT, &modeReg);
 }
 
 
 
 void ICP20100_SetFIFOReadoutMode(ICP20100_Device_t* dev, ICP20100_FIFO_READOUT_MODE range){
     uint8_t modeReg;
     ICP20100_ReadRegister(dev, MODE_SELECT, &modeReg, 1);
     modeReg &= 0b11111100;
     modeReg |=  range;
     dev->settings.fifo_read = range;
     ICP20100_WriteRegister(dev, MODE_SELECT, &modeReg);
 
 }
 
 
 
 void ICP20100_SetUserReadMode(ICP20100_Device_t* dev, ICP20100_USER_READ_MODE range){
 
     dev->settings.user_read_mode = range;
 }
 
 
 
 int8_t ICP20100_ReadFifo(ICP20100_Device_t* dev, uint8_t address, uint8_t *data, uint8_t lenght){
 
     int8_t status;
     status = ICP20100_ReadRegister(dev, address, data, lenght);
     return status;
 
 }
 
 
 
 int8_t ICP20100_DataInFifo(ICP20100_Device_t* dev, float *temp_c, float *press){
 
     uint8_t fifo_fill;
     int8_t status;
     ICP20100_ReadRegister(dev, FIFO_FILL, &fifo_fill, 1);
     fifo_fill &= 0x1F;
 
     int32_t press_raw = 0;
     int32_t temp_raw = 0;
     *temp_c = 0;
     *press = 0;
 
     uint8_t data[6];
 
 
     if(fifo_fill > 0){
 
         if(dev->settings.fifo_read == PRESSURE_FIRST){
 
 
             status = ICP20100_ReadFifo(dev, 0xFA, data, 6);
 
             press_raw = (data[0] << 16) | (data[1] << 8) | data[2];
             temp_raw = (data[3] << 16) | (data[4] << 8) | data[5];
 
 
         }
 
 
 
         if(dev->settings.fifo_read == TEMPERATURE_FIRST){
 
                 status = ICP20100_ReadFifo(dev, 0xFA, data, 6);
 
                 temp_raw = (data[0] << 16) | (data[1] << 8) | data[2];
                 press_raw = (data[3] << 16) | (data[4] << 8) | data[5];
 
                 *temp_c = TEMP_CONVERT_C(temp_raw);
                 *press = PRESS_CONVERT_PA(press_raw);
 
         }
 
 
 
         if(dev->settings.fifo_read == PRESSURE_ONLY){
 
             status = ICP20100_ReadFifo(dev, 0xFD, data, 3);
 
             press_raw = (data[0] << 16) | (data[1] << 8) | data[2];
 
             *press = PRESS_CONVERT_PA(press_raw);
         }
 
 
 
         if(dev->settings.fifo_read == TEMPERATURE_ONLY){
 
             status = ICP20100_ReadFifo(dev, 0xFD, data, 3);
 
             temp_raw = (data[0] << 16) | (data[1] << 8) | data[2];
 
             *temp_c = TEMP_CONVERT_C(temp_raw);
 
         }
     }
 
     return status;
 }
 
 
 
 int8_t ICP20100_DataInRegister(ICP20100_Device_t* dev, float *temp_c, float *press){
 
     uint8_t data[3];
     int8_t status;
 
     status = ICP20100_ReadRegister(dev, TEMP_DATA_0, data, 3);
     int32_t temp_raw = (data[0] << 16) | (data[1] << 8) | data[2];
     *temp_c = TEMP_CONVERT_C(temp_raw);
 
     status = ICP20100_ReadRegister(dev, PRESS_DATA_0, data, 3);
     int32_t press_raw = (data[0] << 16) | (data[1] << 8) | data[2];
     *press = PRESS_CONVERT_PA(press_raw);
 }
 
 
 
 int8_t ICP20100_ReadData(ICP20100_Device_t* dev, float *temp, float *press){
 
     int8_t status;
 
     if(dev->settings.user_read_mode == 0){
 
         status = ICP20100_DataInFifo(dev, temp, press);
 
     }
 
     if(dev->settings.user_read_mode == 1){
 
         status = ICP20100_DataInRegister(dev, temp, press);
     }
 
     return status;
 
 }
 
 
 
 
 
 void ICP20100_FifoFlush(ICP20100_Device_t* dev){
 
     uint8_t flush;
     ICP20100_ReadRegister(dev, &FIFO_FILL, &flush, 1);
     flush |= 0b10000000;
 
     ICP20100_WriteRegister(dev, &FIFO_FILL, &flush);
 }
 
 
 
 

 