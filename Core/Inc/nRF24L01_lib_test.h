/*
 * nRF24L01_lib_test.h
 *
 *  Created on: 11 lis 2020
 *      Author: Khevenin
 */

#ifndef INC_NRF24L01_LIB_TEST_H_
#define INC_NRF24L01_LIB_TEST_H_

/* Unity test libary header */
#include "unity.h"
/* Main nRF24L01 library header */
#include "nRF24L01.h"
#include <stdbool.h>

bool test_StructInit(nRF24L01_struct_t *psNRF24L01);

bool test_DefaultRegistersValue(nRF24L01_struct_t *psNRF24L01);
bool test_WriteReadRegisters(nRF24L01_struct_t *psNRF24L01);

bool test_Power(nRF24L01_struct_t *psNRF24L01);
bool test_Mode(nRF24L01_struct_t *psNRF24L01);
bool test_CRC_Enable(nRF24L01_struct_t *psNRF24L01);
bool test_Interrupts_Enable(nRF24L01_struct_t *psNRF24L01);
bool test_setCRC(nRF24L01_struct_t *psNRF24L01);


#endif /* INC_NRF24L01_LIB_TEST_H_ */