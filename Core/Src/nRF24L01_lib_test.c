/*
 * nRF24L01_lib_test.c
 *
 *  Created on: 11 lis 2020
 *      Author: Khevenin
 */

#include "nRF24L01_lib_test.h"

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
	TEST_ASSERT_EACH_EQUAL_UINT8(DF_RX_ADDR_P0_0, readTab, 5);
	memset(readTab, 0, 5);

	readRegExt(psNRF24L01, RX_ADDR_P1, readTab, 5);
	TEST_ASSERT_EACH_EQUAL_UINT8(DF_RX_ADDR_P1_0, readTab, 5);
	memset(readTab, 0, 5);

	readRegExt(psNRF24L01, TX_ADDR, readTab, 5);
	TEST_ASSERT_EACH_EQUAL_UINT8(DF_TX_ADDR_0, readTab, 5);
	memset(readTab, 0, 5);

	readVar = readReg(psNRF24L01, DYNPD);
	TEST_ASSERT_EQUAL(DF_DYNPD, readVar);

	readVar = readReg(psNRF24L01, FEATURE);
	TEST_ASSERT_EQUAL(DF_FEATURE, readVar);

	return true;
}
