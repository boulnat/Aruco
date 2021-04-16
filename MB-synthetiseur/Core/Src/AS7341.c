/*
 * AS7341.c
 *
 *  Created on: Mar 30, 2021
 *      Author: boulnat
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <main.h>
#include <AS7341.h>

/*!
 *    @brief  Sets up the hardware and initializes I2C
 *    @param  i2c_address
 *            The I2C address to be used.
 *    @return True if initialization was successful, otherwise false.
 */
bool AS7341begin(I2C_HandleTypeDef hi2c1){
	hi2c = hi2c1;
	return 1;
}

bool setASTEP(uint8_t astep_value) {
	uint8_t data[] = {AS7341_ASTEP_L, astep_value};
	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, data, sizeof(data), HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);

	data[0] = AS7341_ASTEP_H;
	data[1] = 0x03;
	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, data, sizeof(data), HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);

	return 1;
}

bool setATIME(uint8_t atime_value) {
	uint8_t data[] = {AS7341_ATIME, atime_value};
	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, data, sizeof(data), HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);

	return 1;
}

bool setGain(as7341_gain_t gain_value) {
	uint8_t data[] = {AS7341_CFG1, gain_value};
	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, data, sizeof(data), HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);

	return 1;
  // AGAIN bitfield is only[0:4] but the rest is empty
}

uint16_t getASTEP() {
	uint8_t regtest[]={AS7341_ASTEP_L};
	uint16_t regRead[1]={0};
	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regtest, 1, HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
	while(HAL_I2C_Master_Receive(&hi2c, 0x72, regRead, sizeof(regRead), HAL_MAX_DELAY)!= HAL_OK);
  return regRead[0];

}

uint8_t getATIME() {
	uint8_t regtest[]={AS7341_ATIME};
	uint8_t regRead[1]={0};
	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regtest, 1, HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
	while(HAL_I2C_Master_Receive(&hi2c, 0x73, regRead, sizeof(regRead), HAL_MAX_DELAY)!= HAL_OK);
  return regRead[0];
}

as7341_gain_t getGain() {
	uint8_t regtest[]={AS7341_CFG1};
	uint8_t regRead[1]={0};
	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regtest, 1, HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
	while(HAL_I2C_Master_Receive(&hi2c, 0x72, regRead, sizeof(regRead), HAL_MAX_DELAY)!= HAL_OK);

  return (as7341_gain_t)regRead[0];
}

long getTINT(){
	  uint16_t astep = getASTEP();
	  uint8_t atime = getATIME();

	  return (atime + 1) * (astep + 1) * 2.78 / 1000;
}

float toBasicCounts(uint16_t raw){
	  float gain_val = 0;
	  as7341_gain_t gain = getGain();
	  switch (gain) {
	  case AS7341_GAIN_0_5X:
	    gain_val = 0.5;
	    break;
	  case AS7341_GAIN_1X:
	    gain_val = 1;
	    break;
	  case AS7341_GAIN_2X:
	    gain_val = 2;
	    break;
	  case AS7341_GAIN_4X:
	    gain_val = 4;
	    break;
	  case AS7341_GAIN_8X:
	    gain_val = 8;
	    break;
	  case AS7341_GAIN_16X:
	    gain_val = 16;
	    break;
	  case AS7341_GAIN_32X:
	    gain_val = 32;
	    break;
	  case AS7341_GAIN_64X:
	    gain_val = 64;
	    break;
	  case AS7341_GAIN_128X:
	    gain_val = 128;
	    break;
	  case AS7341_GAIN_256X:
	    gain_val = 256;
	    break;
	  case AS7341_GAIN_512X:
	    gain_val = 512;
	    break;
	  }
	  return raw / (gain_val * (getATIME() + 1) * (getASTEP() + 1) * 2.78 / 1000);
}

uint16_t readAllChannels(uint16_t *readings_buffer) {
	uint8_t regwrite[]={AS7341_CH0_DATA_L,0x02};
	uint16_t buff = 0;

    for(int i=0; i<12; i++){
        _channel_readings[i]=0;
    }

	setSMUXLowChannels(true);        // Configure SMUX to read low channels
  	enableSpectralMeasurement(true); // Start integration
  	delayForData(0);                 // I'll wait for you for all time

  	readings_buffer=_channel_readings;

  	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regwrite, 1, HAL_MAX_DELAY) != HAL_OK);
  	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);

  	regwrite[0]=AS7341_CH0_DATA_L;
  	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regwrite, sizeof(regwrite), HAL_MAX_DELAY) != HAL_OK);
  	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
  	while(HAL_I2C_Master_Receive(&hi2c, 0x72, (uint8_t *)readings_buffer, 12, HAL_MAX_DELAY)!= HAL_OK);


  setSMUXLowChannels(false);       // Configure SMUX to read high channels
  enableSpectralMeasurement(true); // Start integration
  delayForData(0);                 // I'll wait for you for all time

  regwrite[0]=AS7341_CH0_DATA_L;
  regwrite[1]=0x02;
  while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regwrite, 1, HAL_MAX_DELAY) != HAL_OK);
  while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);

  regwrite[0]=AS7341_CH0_DATA_L;
  while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regwrite, sizeof(regwrite), HAL_MAX_DELAY) != HAL_OK);
  while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
  while(HAL_I2C_Master_Receive(&hi2c, 0x72, (uint8_t *)&readings_buffer[6], 12, HAL_MAX_DELAY)!= HAL_OK);

	//swap MSB and LSB

  for(int i=0; i<12; i++){
	  buff = ((_channel_readings[i] & 0x00FF) << 8) | (_channel_readings[i]>>8);
	  _channel_readings[i] = buff;
  }

  return 1;
}

void delayForData(int waitTime) {
	uint8_t regwrite[]={AS7341_STATUS2};
	uint8_t regRead[1]={0};

	while(regRead[0]>>6!=0x01){
//		HAL_Delay(100);
		while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regwrite, sizeof(regwrite), HAL_MAX_DELAY) != HAL_OK);
		while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
		while(HAL_I2C_Master_Receive(&hi2c, 0x72, regRead, sizeof(regRead), HAL_MAX_DELAY)!= HAL_OK);
		//sprintf(msg, "delayForData = %d\r\n", regRead[0]);
		//HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);
	}
}

uint16_t readChannel(as7341_adc_channel_t channel) {

	uint16_t read[1];
	uint16_t read16bits = 0;
	uint8_t regCh[] = {AS7341_CH0_DATA_L + 2 * channel};

	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regCh, 2, HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
	while(HAL_I2C_Master_Receive(&hi2c, 0x72, read, sizeof(read), HAL_MAX_DELAY)!= HAL_OK);


	read16bits = ((read[0] & 0x00FF) << 8) | read[0]>>8;
	return read16bits;

}

uint16_t getChannel(as7341_color_channel_t channel) {
  return _channel_readings[channel];
}

bool startReading(void){
	  _readingState = AS7341_WAITING_START; // Start the measurement please
	  checkReadingProgress();               // Call the check function to start it
	  return true;
}

bool checkReadingProgress(){
	uint8_t regwrite[]={AS7341_CH0_DATA_L,0x02};
	  if (_readingState == AS7341_WAITING_START) {
	    setSMUXLowChannels(true);        // Configure SMUX to read low channels
	    enableSpectralMeasurement(true); // Start integration
	    _readingState = AS7341_WAITING_LOW;
	    return false;
	  }

	  if (!getIsDataReady() || _readingState == AS7341_WAITING_DONE)
	    return false;

	  if (_readingState == AS7341_WAITING_LOW) // Check of getIsDataRead() is already done
	  {
		  while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regwrite, sizeof(regwrite), HAL_MAX_DELAY) != HAL_OK);
		  while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
		  while(HAL_I2C_Master_Receive(&hi2c, 0x72, (uint8_t *)_channel_readings, 12, HAL_MAX_DELAY)!= HAL_OK);

	    setSMUXLowChannels(false);       // Configure SMUX to read high channels
	    enableSpectralMeasurement(true); // Start integration
	    _readingState = AS7341_WAITING_HIGH;
	    return false;
	  }

	  if (_readingState == AS7341_WAITING_HIGH) // Check of getIsDataRead() is already done
	  {
	    _readingState = AS7341_WAITING_DONE;
	    while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regwrite, sizeof(regwrite), HAL_MAX_DELAY) != HAL_OK);
	    while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
	    while(HAL_I2C_Master_Receive(&hi2c, 0x72, (uint8_t *)&_channel_readings[6], 12, HAL_MAX_DELAY)!= HAL_OK);

	    return true;
	  }

	  return false;
}

bool getAllChannels(uint16_t *readings_buffer){
	for (int i = 0; i < 12; i++){
		readings_buffer[i] = _channel_readings[i];
	}
	return true;
}


uint16_t detectFlickerHz(void){
	  bool isEnabled = true;
	  bool isFdmeasReady = false;

	  // disable everything; Flicker detect, smux, wait, spectral, power
	  disableAll();
	  // re-enable power
	  powerEnable(true);

	  // Write SMUX configuration from RAM to set SMUX chain registers (Write 0x10
	  // to CFG6)
	  setSMUXCommand(AS7341_SMUX_CMD_WRITE);

	  // Write new configuration to all the 20 registers for detecting Flicker
	  FDConfig();

	  // Start SMUX command
	  enableSMUX();

	  // Enable SP_EN bit
	  enableSpectralMeasurement(true);

	  // Enable flicker detection bit
	  writeRegister(byte(AS7341_ENABLE), byte(0x41));
	  delay(500); // SF 2020-08-12 Does this really need to be so long?
	  uint16_t flicker_status = getFlickerDetectStatus();
	  enableFlickerDetection(false);
	  switch (flicker_status) {
	  case 44:
	    return 1;
	  case 45:
	    return 100;
	  case 46:
	    return 120;
	  default:
	    return 0;
	  }
}

void setup_F1F4_Clear_NIR() {
  // SMUX Config for F1,F2,F3,F4,NIR,Clear
  writeRegister((0x00), (0x30)); // F3 left set to ADC2
  writeRegister((0x01), (0x01)); // F1 left set to ADC0
  writeRegister((0x02), (0x00)); // Reserved or disabled
  writeRegister((0x03), (0x00)); // F8 left disabled
  writeRegister((0x04), (0x00)); // F6 left disabled
  writeRegister((0x05), (0x42)); // F4 left connected to ADC3/f2 left connected to ADC1
  writeRegister((0x06), (0x00)); // F5 left disbled
  writeRegister((0x07), (0x00)); // F7 left disbled
  writeRegister((0x08), (0x50)); // CLEAR connected to ADC4
  writeRegister((0x09), (0x00)); // F5 right disabled
  writeRegister((0x0A), (0x00)); // F7 right disabled
  writeRegister((0x0B), (0x00)); // Reserved or disabled
  writeRegister((0x0C), (0x20)); // F2 right connected to ADC1
  writeRegister((0x0D), (0x04)); // F4 right connected to ADC3
  writeRegister((0x0E), (0x00)); // F6/F8 right disabled
  writeRegister((0x0F), (0x30)); // F3 right connected to AD2
  writeRegister((0x10), (0x01)); // F1 right connected to AD0
  writeRegister((0x11), (0x50)); // CLEAR right connected to AD4
  writeRegister((0x12), (0x00)); // Reserved or disabled
  writeRegister((0x13), (0x06)); // NIR connected to ADC5
}

void setup_F5F8_Clear_NIR() {
  // SMUX Config for F5,F6,F7,F8,NIR,Clear
  writeRegister((0x00), (0x00)); // F3 left disable
  writeRegister((0x01), (0x00)); // F1 left disable
  writeRegister((0x02), (0x00)); // reserved/disable
  writeRegister((0x03), (0x40)); // F8 left connected to ADC3
  writeRegister((0x04), (0x02)); // F6 left connected to ADC1
  writeRegister((0x05), (0x00)); // F4/ F2 disabled
  writeRegister((0x06), (0x10)); // F5 left connected to ADC0
  writeRegister((0x07), (0x03)); // F7 left connected to ADC2
  writeRegister((0x08), (0x50)); // CLEAR Connected to ADC4
  writeRegister((0x09), (0x10)); // F5 right connected to ADC0
  writeRegister((0x0A), (0x03)); // F7 right connected to ADC2
  writeRegister((0x0B), (0x00)); // Reserved or disabled
  writeRegister((0x0C), (0x00)); // F2 right disabled
  writeRegister((0x0D), (0x00)); // F4 right disabled
  writeRegister((0x0E), (0x24)); // F8 right connected to ADC2/ F6 right connected to ADC1
  writeRegister((0x0F), (0x00)); // F3 right disabled
  writeRegister((0x10), (0x00)); // F1 right disabled
  writeRegister((0x11), (0x50)); // CLEAR right connected to AD4
  writeRegister((0x12), (0x00)); // Reserved or disabled
  writeRegister((0x13), (0x06)); // NIR connected to ADC5
}

void powerEnable(bool enable_power){
	//POWER enable true
	uint8_t regWrite[]={AS7341_ENABLE,0x01}; //PON to 1
	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regWrite, sizeof(regWrite), HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
}

bool enableSpectralMeasurement(bool enable_measurement) {
	uint8_t regwrite[]={AS7341_ENABLE,0x01};
	//uint8_t regRead[1]={0};

	if(enable_measurement==0){
		regwrite[1]=0x01;
		while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regwrite, sizeof(regwrite), HAL_MAX_DELAY) != HAL_OK);
		while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
	}
	else{
		regwrite[1]=0x03;
		while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regwrite, sizeof(regwrite), HAL_MAX_DELAY) != HAL_OK);
		while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
	}
  return 1;
}

bool setHighThreshold(uint16_t high_threshold){
	uint8_t regWrite[]={AS7341_SP_HIGH_TH_L,high_threshold}; //PON to 1
	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regWrite, sizeof(regWrite), HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
	return 1;
}

bool setLowThreshold(uint16_t low_threshold){
	uint8_t regWrite[]={AS7341_SP_LOW_TH_L,low_threshold}; //PON to 1
	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regWrite, sizeof(regWrite), HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
	return 1;
}

uint16_t getHighThreshold(void){
	uint8_t regtest[]={AS7341_SP_HIGH_TH_L};
	uint16_t regRead[1]={0};
	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regtest, 1, HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
	while(HAL_I2C_Master_Receive(&hi2c, 0x72, regRead, sizeof(regRead), HAL_MAX_DELAY)!= HAL_OK);
	return regRead[0];
}

uint16_t getLowThreshold(void){
	uint8_t regtest[]={AS7341_SP_LOW_TH_L};
	uint16_t regRead[1]={0};
	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regtest, 1, HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
	while(HAL_I2C_Master_Receive(&hi2c, 0x72, regRead, sizeof(regRead), HAL_MAX_DELAY)!= HAL_OK);
	return regRead[0];
}

bool enableSpectralInterrupt(bool enable_int);
bool enableSystemInterrupt(bool enable_int);

bool setAPERS(as7341_int_cycle_count_t cycle_count);
bool setSpectralThresholdChannel(as7341_adc_channel_t channel);

uint8_t getInterruptStatus(void);
bool clearInterruptStatus(void);

bool spectralInterruptTriggered(void);
uint8_t spectralInterruptSource(void);
bool spectralLowTriggered(void);
bool spectralHighTriggered(void);

bool enableLED(bool enable_led);
bool setLEDCurrent(uint16_t led_current_ma);

void disableAll(void){
	//POWER disable
	uint8_t regWrite[]={AS7341_ENABLE,0x00}; //PON to 1
	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regWrite, sizeof(regWrite), HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
}

bool getIsDataReady();
bool setBank(bool low); // low true gives access to 0x60 to 0x74

as7341_gpio_dir_t getGPIODirection(void);
bool setGPIODirection(as7341_gpio_dir_t gpio_direction);
bool getGPIOInverted(void);
bool setGPIOInverted(bool gpio_inverted);
bool getGPIOValue(void);
bool setGPIOValue(bool);

/*!  @brief Initializer for post i2c/spi init
 *   @param sensor_id Optional unique ID for the sensor set
 *   @returns True if chip identified and initialized
 */
bool AS7341init(int32_t sensor_id){
    //POWER enable true
    uint8_t regWrite[]={AS7341_ENABLE,0x01}; //PON to 1
	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regWrite, sizeof(regWrite), HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
	return 1;
}

bool enableSMUX() {
	uint8_t regwrite[]={AS7341_ENABLE,0x19};
	uint8_t regRead[1]={0};

	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regwrite, sizeof(regwrite), HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);


	regwrite[0]=AS7341_ENABLE;//0xA9 to AS7341_ENABLE
	regRead[0]=0;
	while(regRead[0]==0){
		while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regwrite, 1, HAL_MAX_DELAY) != HAL_OK);
		while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
		while(HAL_I2C_Master_Receive(&hi2c, 0x72, regRead, sizeof(regRead), HAL_MAX_DELAY)!= HAL_OK);

		//sprintf(msg, "enableSMUX = %d\r\n", regRead[0]);
		//HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);
	}

    return 1;
}

bool enableFlickerDetection(bool enable_fd){
	uint8_t regWrite[]={AS7341_ENABLE,0x40}; //PON to 1
	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regWrite, sizeof(regWrite), HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
	return 1;
}

void FDConfig(void){
	  // SMUX Config for Flicker- register (0x13)left set to ADC6 for flicker
	  // detection
	  writeRegister((0x00), (0x00)); // disabled
	  writeRegister((0x01), (0x00)); // disabled
	  writeRegister((0x02), (0x00)); // reserved/disabled
	  writeRegister((0x03), (0x00)); // disabled
	  writeRegister((0x04), (0x00)); // disabled
	  writeRegister((0x05), (0x00)); // disabled
	  writeRegister((0x06), (0x00)); // disabled
	  writeRegister((0x07), (0x00)); // disabled
	  writeRegister((0x08), (0x00)); // disabled
	  writeRegister((0x09), (0x00)); // disabled
	  writeRegister((0x0A), (0x00)); // disabled
	  writeRegister((0x0B), (0x00)); // Reserved or disabled
	  writeRegister((0x0C), (0x00)); // disabled
	  writeRegister((0x0D), (0x00)); // disabled
	  writeRegister((0x0E), (0x00)); // disabled
	  writeRegister((0x0F), (0x00)); // disabled
	  writeRegister((0x10), (0x00)); // disabled
	  writeRegister((0x11), (0x00)); // disabled
	  writeRegister((0x12), (0x00)); // Reserved or disabled
	  writeRegister((0x13), (0x60)); // Flicker connected to ADC5 to left of 0x13
}

// maybe return a typedef enum
/**
 * @brief Returns the flicker detection status
 *
 * @return int8_t
 */
int8_t getFlickerDetectStatus(void){
	uint8_t read[1];
	uint8_t regCh[] = {AS7341_FD_STATUS};

	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regCh, 2, HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
	while(HAL_I2C_Master_Receive(&hi2c, 0x72, read, sizeof(read), HAL_MAX_DELAY)!= HAL_OK);

	return read[0];
}

bool setSMUXCommand(as7341_smux_cmd_t command) {
	uint8_t regwrite[]={AS7341_CFG6,0x10}; //should be command << to something
	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, regwrite, sizeof(regwrite), HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);
	return 1;
}

void writeRegister(uint8_t addr, uint8_t val) {
	uint8_t data[] = {addr, val};
	while(HAL_I2C_Master_Transmit(&hi2c, 0x72, data, sizeof(data), HAL_MAX_DELAY) != HAL_OK);
	while(HAL_I2C_IsDeviceReady(&hi2c,0x72,10,200)!=HAL_OK);

}


void setSMUXLowChannels(bool f1_f4) {
  enableSpectralMeasurement(false);
  setSMUXCommand(AS7341_SMUX_CMD_WRITE);
  if (f1_f4) {
    setup_F1F4_Clear_NIR();
  } else {
    setup_F5F8_Clear_NIR();
  }
  enableSMUX(&hi2c);
}













