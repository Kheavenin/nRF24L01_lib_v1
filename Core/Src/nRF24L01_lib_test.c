/*
 * nRF24L01_lib_test.c
 *
 *  Created on: 11 lis 2020
 *      Author: Khevenin
 */

#include "nRF24L01_lib_test.h"
/* Use after nRF_Init */
bool test_StructInit(nRF24L01_struct_t *psNRF24L01) {
	return true;
}
bool test_ReadDefaultRegistersValue(nRF24L01_struct_t *psNRF24L01) {
	uint8_t readVar = 0;
	uint8_t i;
	uint8_t readTab[5];
	memset(readTab, 0, 5);
	
	readVar = readReg(psNRF24L01, CONFIG);
	TEST_ASSERT_EQUAL(DF_CONFIG, readVar);

	readVar = readReg(psNRF24L01, EN_AA);
	TEST_ASSERT_EQUAL(DF_EN_AA, readVar);

	readVar = readReg(psNRF24L01, EN_RXADDR);
	TEST_ASSERT_EQUAL(DF_RXADDR, readVar);

	readVar = readReg(psNRF24L01, SETUP_AW);
	TEST_ASSERT_EQUAL(DF_SETUP_AW, readVar);

	readVar = readReg(psNRF24L01, SETUP_RETR);
	TEST_ASSERT_EQUAL(DF_SETUP_RETR, readVar);

	readVar = readReg(psNRF24L01, RF_CH);
	TEST_ASSERT_EQUAL(DF_RF_CH, readVar);

	readVar = readReg(psNRF24L01, RF_SETUP);
	TEST_ASSERT_EQUAL(DF_RF_SETUP, readVar);

	readVar = readReg(psNRF24L01, OBSERVE_TX);
	TEST_ASSERT_EQUAL(DF_OBSERVE_TX, readVar);

	for (i = 0; i < 6; i++) {
		readVar = readReg(psNRF24L01, RX_PW_P0 + i);
		TEST_ASSERT_EQUAL(DF_RX_PW_P0, readVar);
	}

	readRegExt(psNRF24L01, RX_ADDR_P0, readTab, 5);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(DF_RX_ADDR_P0_0, readTab, 5);
	memset(readTab, 0, 5);

	readRegExt(psNRF24L01, RX_ADDR_P1, readTab, 5);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(DF_RX_ADDR_P1_0, readTab, 5);
	memset(readTab, 0, 5);

	readRegExt(psNRF24L01, TX_ADDR, readTab, 5);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(DF_TX_ADDR_0, readTab, 5);
	memset(readTab, 0, 5);

	readVar = readReg(psNRF24L01, DYNPD);
	TEST_ASSERT_EQUAL(DF_DYNPD, readVar);

	readVar = readReg(psNRF24L01, FEATURE);
	TEST_ASSERT_EQUAL(DF_FEATURE, readVar);

	return true;
}
bool test_WriteReadRegisters(nRF24L01_struct_t *psNRF24L01) {
	uint8_t readVar = 0;
	readVar = readReg(psNRF24L01, CONFIG);
	writeReg(psNRF24L01, CONFIG, 0x0F);

	readVar = readReg(psNRF24L01, CONFIG);
	TEST_ASSERT_EQUAL_UINT8(0x0F, readVar);
	writeReg(psNRF24L01, CONFIG, DF_CONFIG);

	return true;
}

bool test_Power(nRF24L01_struct_t *psNRF24L01) {
	pwrUp(psNRF24L01);
	uint8_t readVar = readReg(psNRF24L01, CONFIG);
	TEST_ASSERT_BITS(0x02, 0x02, readVar);

	pwrDown(psNRF24L01);
	readVar = readReg(psNRF24L01, CONFIG);
	TEST_ASSERT_BITS(0x00, 0x00, readVar);

	pwrUp(psNRF24L01);
	return true;
}
bool test_Mode(nRF24L01_struct_t *psNRF24L01) {

	modeRX(psNRF24L01);
	uint8_t readVar = readReg(psNRF24L01, CONFIG);
	TEST_ASSERT_BITS(0x01, 0x01, readVar);

	modeTX(psNRF24L01);
	readVar = readReg(psNRF24L01, CONFIG);
	TEST_ASSERT_BITS(0x01, 0x00, readVar);

	modeRX(psNRF24L01);
	readVar = readReg(psNRF24L01, CONFIG);
	TEST_ASSERT_BITS(0x01, 0x01, readVar);

	return true;
}
bool test_CRC_Enable(nRF24L01_struct_t *psNRF24L01) {
	enableCRC(psNRF24L01);
	uint8_t readVar = readReg(psNRF24L01, CONFIG);
	TEST_ASSERT_BITS(0x08, 0x08, readVar);

	disableCRC(psNRF24L01);
	readVar = readReg(psNRF24L01, CONFIG);
	TEST_ASSERT_BITS(0x08, 0x00, readVar);

	enableCRC(psNRF24L01);
	readVar = readReg(psNRF24L01, CONFIG);
	TEST_ASSERT_BITS(0x08, 0x08, readVar);

	return true;
}
bool test_Interrupts_Enable(nRF24L01_struct_t *psNRF24L01) {

	writeReg(psNRF24L01, CONFIG, 0x07);

	enableTXinterrupt(psNRF24L01);
	enableRXinterrupt(psNRF24L01);
	enableMaxRTinterrupt(psNRF24L01);

	uint8_t readVar = readReg(psNRF24L01, CONFIG);
	TEST_ASSERT_BITS(0x70, 0x70, readVar);

	disableTXinterrupt(psNRF24L01);
	disableRXinterrupt(psNRF24L01);
	disableMaxRTinterrupt(psNRF24L01);

	readVar = readReg(psNRF24L01, CONFIG);
	TEST_ASSERT_BITS(0x0, 0x00, readVar);

	enableTXinterrupt(psNRF24L01);
	enableRXinterrupt(psNRF24L01);
	enableMaxRTinterrupt(psNRF24L01);

	readVar = readReg(psNRF24L01, CONFIG);
	TEST_ASSERT_BITS(0x70, 0x70, readVar);

	return true;
}
bool test_setCRC(nRF24L01_struct_t *psNRF24L01) {
	setCRC(psNRF24L01, CRC_8_bits);
	uint8_t readVar = readReg(psNRF24L01, CONFIG);
	TEST_ASSERT_BITS(0x04, 0x00, readVar);

	setCRC(psNRF24L01, CRC_16_bits);
	readVar = readReg(psNRF24L01, CONFIG);
	TEST_ASSERT_BITS(0x04, 0x04, readVar);

	setCRC(psNRF24L01, CRC_8_bits);
	readVar = readReg(psNRF24L01, CONFIG);
	TEST_ASSERT_BITS(0x04, 0x00, readVar);

	return true;
}
bool test_EN_AA(nRF24L01_struct_t *psNRF24L01) {
	uint8_t i, readVar;
	for (i = 0; i < 6; ++i) {
		disableAutoAckPipe(psNRF24L01, i);
	}
	readVar = readReg(psNRF24L01, EN_AA);
	TEST_ASSERT_BITS(0x3F, 0x00, readVar);
	for (i = 0; i < 6; ++i) {
		enableAutoAckPipe(psNRF24L01, i);
	}
	readVar = readReg(psNRF24L01, EN_AA);
	TEST_ASSERT_BITS(0x3F, 0x3F, readVar);

	return true;
}
bool test_EN_RXADDR(nRF24L01_struct_t *psNRF24L01) {
	uint8_t i, readVar;

	readVar = readReg(psNRF24L01, EN_RXADDR);
	TEST_ASSERT_BITS(0x3F, 0x03, readVar);

	for (i = 0; i < 6; ++i) {
		enableRxAddr(psNRF24L01, i);
	}
	readVar = readReg(psNRF24L01, EN_RXADDR);
	TEST_ASSERT_BITS(0x3F, 0x3F, readVar);

	for (i = 0; i < 6; ++i) {
		disableRxAddr(psNRF24L01, i);
	}
	readVar = readReg(psNRF24L01, EN_RXADDR);
	TEST_ASSERT_BITS(0x3F, 0x00, readVar);

	enableRxAddr(psNRF24L01, 0);
	enableRxAddr(psNRF24L01, 1);

	return true;
}
bool test_SetupAW(nRF24L01_struct_t *psNRF24L01) {
	uint8_t readVar;
	readVar = readReg(psNRF24L01, SETUP_AW);
	TEST_ASSERT_BITS(0x03, 0x03, readVar);

	setAddrWidth(psNRF24L01, shortWidth);
	readVar = readReg(psNRF24L01, SETUP_AW);
	TEST_ASSERT_BITS(0x03, 0x01, readVar);

	setAddrWidth(psNRF24L01, mediumWidth);
	readVar = readReg(psNRF24L01, SETUP_AW);
	TEST_ASSERT_BITS(0x03, 0x02, readVar);

	setAddrWidth(psNRF24L01, longWidth);
	readVar = readReg(psNRF24L01, SETUP_AW);
	TEST_ASSERT_BITS(0x03, 0x03, readVar);

	return true;
}
bool test_ARC(nRF24L01_struct_t *psNRF24L01) {
	uint8_t readVar;
	readVar = readReg(psNRF24L01, SETUP_RETR);
	TEST_ASSERT_BITS(0x0F, 0x03, readVar);

	setAutoRetrCount(psNRF24L01, 0x04);
	readVar = readReg(psNRF24L01, SETUP_RETR);
	TEST_ASSERT_BITS(0x0F, 0x04, readVar);

	setAutoRetrCount(psNRF24L01, 0x07);
	readVar = readReg(psNRF24L01, SETUP_RETR);
	TEST_ASSERT_BITS(0x0F, 0x07, readVar);

	return true;
}
bool test_ARD(nRF24L01_struct_t *psNRF24L01) {
	uint8_t readVar;
	readVar = readReg(psNRF24L01, SETUP_RETR);
	TEST_ASSERT_BITS(0xF0, 0x00, readVar);

	setAutoRetrDelay(psNRF24L01, 0xF0);
	readVar = readReg(psNRF24L01, SETUP_RETR);
	TEST_ASSERT_BITS(0xF0, 0xF0, readVar);

	setAutoRetrDelay(psNRF24L01, 0x30);
	readVar = readReg(psNRF24L01, SETUP_RETR);
	TEST_ASSERT_BITS(0xF0, 0x30, readVar);

	return true;
}
bool test_RF_CH(nRF24L01_struct_t *psNRF24L01) {
	uint8_t readVar;
	readVar = readReg(psNRF24L01, RF_CH);
	TEST_ASSERT_BITS(0x7F, 0x02, readVar);

	setChannel(psNRF24L01, 0x0F);
	readVar = readReg(psNRF24L01, RF_CH);
	TEST_ASSERT_BITS(0x7F, 0x0F, readVar);

	setChannel(psNRF24L01, 0x04);
	readVar = readReg(psNRF24L01, RF_CH);
	TEST_ASSERT_BITS(0x7F, 0x04, readVar);

	return true;
}
bool test_RFpower(nRF24L01_struct_t *psNRF24L01) {
	uint8_t readVar;
	readVar = readReg(psNRF24L01, RF_SETUP);
	TEST_ASSERT_BITS(0x06, 0x06, readVar);

	setRFpower(psNRF24L01, RF_PWR_18dBm);
	readVar = readReg(psNRF24L01, RF_SETUP);
	TEST_ASSERT_BITS(0x06, 0x00, readVar);

	setRFpower(psNRF24L01, RF_PWR_6dBm);
	readVar = readReg(psNRF24L01, RF_SETUP);
	TEST_ASSERT_BITS(0x06, 0x04, readVar);

	return true;
}
bool test_DataRate(nRF24L01_struct_t *psNRF24L01) {
	uint8_t readVar;
	readVar = readReg(psNRF24L01, RF_SETUP);
	TEST_ASSERT_BITS(0x14, 0x04, readVar);

	setDataRate(psNRF24L01, RF_DataRate_1M);
	readVar = readReg(psNRF24L01, RF_SETUP);
	TEST_ASSERT_BITS(0x14, 0x00, readVar);

	setDataRate(psNRF24L01, RF_DataRate_2M);
	readVar = readReg(psNRF24L01, RF_SETUP);
	TEST_ASSERT_BITS(0x14, 0x04, readVar);

	setDataRate(psNRF24L01, RF_DataRate_250);
	readVar = readReg(psNRF24L01, RF_SETUP);
	TEST_ASSERT_BITS(0x14, 0x14, readVar);

	return true;
}
