/*
 * nRF24L01.h
 *
 *  Created on: 1 lis 2020
 *  @Author: Khevenin
 *  @Name: nRF24L01 library
 *  @Version: v2.00
 *
 *  Reafactored structures and functions.
 *  Implemented SPI interrupts and DMA services.
 *
 */

#include "nRF24L01.h"

/**
 * @Static function for init structures
 */
static void nRF24L01_hardware_Init(nRF24L01_struct_t *psNRF24L01,
		SPI_HandleTypeDef *HAL_SPIx, TIM_HandleTypeDef *HAL_TIMx,
		GPIO_TypeDef *HAL_GPIO_CSN, uint16_t HAL_GPIO_Pin_CSN,
		GPIO_TypeDef *HAL_GPIO_CE, uint16_t HAL_GPIO_Pin_CE);
static void nRF24L01_settings_Init(nRF24L01_struct_t *psNRF24L01);
static void nRF24L01_addresss_Init(nRF24L01_struct_t *psNRF24L01);
static void nRF24L01_FIFO_Init(nRF24L01_struct_t *psNRF24L01);
static void nRF24L01_status_Init(nRF24L01_struct_t *psNRF24L01);
static void nRF24L01_buffers_Init(nRF24L01_struct_t *psNRF24L01);

void delayUs(nRF24L01_struct_t *psNRF24L01, uint16_t time);
static uint8_t checkPipe(uint8_t pipe);
static void csnLow(nRF24L01_struct_t *psNRF24L01);
static void csnHigh(nRF24L01_struct_t *psNRF24L01);
static void ceLow(nRF24L01_struct_t *psNRF24L01);
static void ceHigh(nRF24L01_struct_t *psNRF24L01);

static uint8_t readBit(nRF24L01_struct_t *psNRF24L01, uint8_t addr,
		bitNum_t bit);
static void resetBit(nRF24L01_struct_t *psNRF24L01, uint8_t addr, bitNum_t bit);
static void setBit(nRF24L01_struct_t *psNRF24L01, uint8_t addr, bitNum_t bit);

/**
 * @Main init function
 */
bool nRF_Init(nRF24L01_struct_t *psNRF24L01, SPI_HandleTypeDef *HAL_SPIx,
		TIM_HandleTypeDef *HAL_TIMx, GPIO_TypeDef *HAL_GPIO_CSN,
		uint16_t HAL_GPIO_Pin_CSN, GPIO_TypeDef *HAL_GPIO_CE,
		uint16_t HAL_GPIO_Pin_CE) {
	if (psNRF24L01 == NULL || HAL_SPIx == NULL || HAL_TIMx == NULL
			|| HAL_GPIO_CSN == NULL || HAL_GPIO_CE == NULL) {
		return false;
	}

	nRF24L01_hardware_Init(psNRF24L01, HAL_SPIx, HAL_TIMx, HAL_GPIO_CSN,
			HAL_GPIO_Pin_CSN, HAL_GPIO_CE, HAL_GPIO_Pin_CE);
	nRF24L01_settings_Init(psNRF24L01);
	nRF24L01_addresss_Init(psNRF24L01);
	nRF24L01_FIFO_Init(psNRF24L01);
	nRF24L01_status_Init(psNRF24L01);
	nRF24L01_buffers_Init(psNRF24L01);

	return true;
}

/* Elementary functions  nRf24L01+  */
/* Read and write registers funtions's */
uint8_t readReg(nRF24L01_struct_t *psNRF24L01, uint8_t addr) {
	uint8_t command = R_REGISTER | addr;
	uint8_t data;

	csnLow(psNRF24L01);
#if SPI_BLOCKING_MODE
	HAL_SPI_Transmit((psNRF24L01->hardware_struct.nRFspi), &command, sizeof(command), SPI_TIMEOUT);
	delayUs(psNRF24L01, 50);
	HAL_SPI_Receive((psNRF24L01->hardware_struct.nRFspi), &data, 1, SPI_TIMEOUT);
#endif
#if SPI_IT_MODE
	HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));
	HAL_SPI_Receive_IT(psNRF24L01->hardware_struct.nRFspi, &data, sizeof(data));
#endif
#if SPI_DMA_MODE
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command,
			sizeof(command));
	HAL_SPI_Receive_DMA(psNRF24L01->hardware_struct.nRFspi, &data, 1);
#endif
	csnHigh(psNRF24L01);

	return data;
}
void writeReg(nRF24L01_struct_t *psNRF24L01, uint8_t addr, uint8_t val) {
	uint8_t command = W_REGISTER | addr;
	uint8_t data = val;

	csnLow(psNRF24L01);
#if SPI_BLOCKING_MODE
	HAL_SPI_Transmit((psNRF24L01->hardware_struct.nRFspi), &command, sizeof(command), SPI_TIMEOUT);
	delayUs(psNRF24L01, 50);
	HAL_SPI_Transmit((psNRF24L01->hardware_struct.nRFspi), &data, sizeof(data), SPI_TIMEOUT);
#endif
#if SPI_IT_MODE
	HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));
	HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, &data, sizeof(data));
#endif
#if SPI_DMA_MODE
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command,
			sizeof(command));
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &data,
			sizeof(data));
#endif
	csnHigh(psNRF24L01);
}

/* Extended read and write functions - R/W few registers */
void readRegExt(nRF24L01_struct_t *psNRF24L01, uint8_t addr, uint8_t *pBuf,
		size_t bufSize) {
	if (psNRF24L01 != NULL && pBuf != NULL && bufSize > 0) {
		uint8_t command = R_REGISTER | (addr);	//set command

		csnLow(psNRF24L01);
#if SPI_BLOCKING_MODE
		HAL_SPI_Transmit(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command), SPI_TIMEOUT);     //transmit command
		delayUs(psNRF24L01, 50);
		HAL_SPI_Receive(psNRF24L01->hardware_struct.nRFspi, (pBuf + i), sizeof(uint8_t)), SPI_TIMEOUT);   //receive data
#endif
#if SPI_IT_MODE
		HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));		//transmit command
		HAL_SPI_Receive_IT(psNRF24L01->hardware_struct.nRFspi, (pBuf + i), sizeof(uint8_t));		//receive data
#endif
#if SPI_DMA_MODE
		HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command,
				sizeof(command));	//transmit command
		HAL_Delay(1);
		HAL_SPI_Receive_DMA(psNRF24L01->hardware_struct.nRFspi, (pBuf),
				sizeof(bufSize));	//receive data
#endif
		csnHigh(psNRF24L01);
	}
}
void writeRegExt(nRF24L01_struct_t *psNRF24L01, uint8_t addr, uint8_t *pBuf,
		size_t bufSize) {
	if (psNRF24L01 != NULL && pBuf != NULL && bufSize > 0) {
		uint8_t command = W_REGISTER | addr;	//set command

		csnLow(psNRF24L01);
#if SPI_BLOCKING_MODE
		HAL_SPI_Transmit(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command), SPI_TIMEOUT);     //transmit command
		delayUs(psNRF24L01, 50);
		HAL_SPI_Transmit(psNRF24L01->hardware_struct.nRFspi, (pBuf + i), sizeof(uint8_t)), SPI_TIMEOUT);   //receive data
#endif
#if SPI_IT_MODE
		HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));		//transmit command
		HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, (pBuf + i), sizeof(uint8_t));		//receive data
#endif
#if SPI_DMA_MODE
		HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command,
				sizeof(command));	//transmit command
		HAL_Delay(1);
		HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, pBuf,
				sizeof(bufSize));	//transmit data
#endif
		csnHigh(psNRF24L01);
	}
}

/* Payload's functions */
uint8_t readRxPayload(nRF24L01_struct_t *psNRF24L01, uint8_t *pBuf,
		size_t bufSize) {
	if (bufSize < 1)
		return ERR_CODE;
	if (bufSize > 32)
		bufSize = 32;

	uint8_t command = R_RX_PAYLOAD; //set command mask

	csnLow(psNRF24L01);

#if SPI_BLOCKING_MODE
	 HAL_SPI_Transmit(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command), SPI_TIMEOUT);
	 delayUs(psNRF24L01, 50);
	 HAL_SPI_Receive(psNRF24L01->hardware_struct.nRFspi, pBuf, bufSize, SPI_TIMEOUT);
	 #endif
#if SPI_IT_MODE
	 HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));
	 HAL_SPI_Receive_IT(psNRF24L01->hardware_struct.nRFspi, pBuf, bufSize, &data, sizeof(data));
	 #endif
#if SPI_DMA_MODE
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command,
			sizeof(command));	//send command
	HAL_SPI_Receive_DMA(psNRF24L01->hardware_struct.nRFspi, pBuf, bufSize); //read payload
#endif

	csnHigh(psNRF24L01);

	return OK_CODE;
}
uint8_t writeTxPayload(nRF24L01_struct_t *psNRF24L01, uint8_t *pBuf,
		size_t bufSize) {
	if (bufSize < 1)
		return ERR_CODE;
	if (bufSize > 32)
		bufSize = 32;

	uint8_t command = W_TX_PAYLOAD; //set command mask

	csnLow(psNRF24L01);
#if SPI_BLOCKING_MODE
	 HAL_SPI_Transmit(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command), SPI_TIMEOUT);
	 delayUs(psNRF24L01, 50);
	 HAL_SPI_Transmit(psNRF24L01->hardware_struct.nRFspi, pBuf, bufSize, SPI_TIMEOUT);
	 #endif
#if SPI_IT_MODE
	 HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));
	 HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, pBuf, bufSize);
	 #endif
#if SPI_DMA_MODE
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command,
			sizeof(command)); //send command
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, pBuf, bufSize); //write payload
#endif
	csnHigh(psNRF24L01);

	return OK_CODE;
}
uint8_t writeTxPayloadAck(nRF24L01_struct_t *psNRF24L01, uint8_t *pBuf,
		size_t bufSize) {
	if (bufSize < 1)
		return ERR_CODE;
	if (bufSize > 32)
		bufSize = 32;

	uint8_t command = W_ACK_PAYLOAD; //set command mask

	csnLow(psNRF24L01);
#if SPI_BLOCKING_MODE
	 HAL_SPI_Transmit(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command), SPI_TIMEOUT);
	 delayUs(psNRF24L01, 50);
	 HAL_SPI_Transmit(psNRF24L01->hardware_struct.nRFspi, pBuf, bufSize, SPI_TIMEOUT);
	 #endif
#if SPI_IT_MODE
	 HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));
	 HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, pBuf, bufSize);
	 #endif
#if SPI_DMA_MODE
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command,
			sizeof(command)); //send command
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, pBuf, bufSize); //write payload
#endif
	csnHigh(psNRF24L01);

	return OK_CODE;
}
uint8_t writeTxPayloadNoAck(nRF24L01_struct_t *psNRF24L01, uint8_t *pBuf,
		size_t bufSize) {
	if (bufSize < 1)
		return ERR_CODE;
	if (bufSize > 32)
		bufSize = 32;

	uint8_t command = W_TX_PAYLOAD_NO_ACK; //set command mask

	csnLow(psNRF24L01);
#if SPI_BLOCKING_MODE
	 HAL_SPI_Transmit(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command), SPI_TIMEOUT);
	 delayUs(psNRF24L01, 50);
	 HAL_SPI_Transmit(psNRF24L01->hardware_struct.nRFspi, pBuf, bufSize, SPI_TIMEOUT);
	 #endif
#if SPI_IT_MODE
	 HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));
	 HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, pBuf, bufSize);
	 #endif
#if SPI_DMA_MODE
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command,
			sizeof(command)); //send command
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, pBuf, bufSize);//write payload
#endif

	csnHigh(psNRF24L01);

	return OK_CODE;
}

/* Read payload lenght when DPL enable */
uint8_t readDynamicPayloadWidth(nRF24L01_struct_t *psNRF24L01) {
	uint8_t command = R_RX_PL_WID; //set command mask
	uint8_t width;

	csnLow(psNRF24L01);
	 #if SPI_BLOCKING_MODE
	 HAL_SPI_Transmit(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command), SPI_TIMEOUT);
	 delayUs(psNRF24L01, 50);
	 HAL_SPI_Receive(psNRF24L01->hardware_struct.nRFspi, &width, sizeof(width),SPI_TIMEOUT);
	 #endif
	 #if SPI_IT_MODE
	 HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));
	 HAL_SPI_Receive_IT(psNRF24L01->hardware_struct.nRFspi, &width, sizeof(width));
	 #endif
	 #if SPI_DMA_MODE
	 HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command)); //send command
	 HAL_SPI_Receive_DMA(psNRF24L01->hardware_struct.nRFspi, &width, sizeof(width)); //read payload width
	 #endif
	csnHigh(psNRF24L01);

	return width;
}

/* Flush functions */
uint8_t flushTx(nRF24L01_struct_t *psNRF24L01) {
	uint8_t command = FLUSH_TX; //set command mask

	csnLow(psNRF24L01);
#if SPI_BLOCKING_MODE
	HAL_SPI_Transmit(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command), SPI_TIMEOUT);
#endif
#if SPI_IT_MODE
	HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));
#endif
#if SPI_DMA_MODE
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command,
			sizeof(command)); //Flush TX
#endif
	csnHigh(psNRF24L01);

	if (!readBit(psNRF24L01, FIFO_STATUS, bit4)) { //check FIFO status
		psNRF24L01->fifo_struct.txEmpty = 0;
		return ERR_CODE;
	}
	psNRF24L01->fifo_struct.txEmpty = 1;

	return OK_CODE;
}
uint8_t flushRx(nRF24L01_struct_t *psNRF24L01) {
	uint8_t command = FLUSH_RX; //set command mask

	csnLow(psNRF24L01);
#if SPI_BLOCKING_MODE
	HAL_SPI_Transmit(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command), SPI_TIMEOUT);
#endif
#if SPI_IT_MODE
	HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));
#endif
#if SPI_DMA_MODE
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command,
			sizeof(command)); //Flush RX
#endif
	csnHigh(psNRF24L01);

	if (!readBit(psNRF24L01, FIFO_STATUS, bit0)) { //check FIFO status
		psNRF24L01->fifo_struct.rxEmpty = 0;
		return ERR_CODE;
	}
	psNRF24L01->fifo_struct.rxEmpty = 1;

	return OK_CODE;
}

/* Others */
void reuseTxPayload(nRF24L01_struct_t *psNRF24L01) {
	uint8_t command = REUSE_TX_PL; //set command mask

	csnLow(psNRF24L01);
#if SPI_BLOCKING_MODE
	HAL_SPI_Transmit(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command), SPI_TIMEOUT);
#endif
#if SPI_IT_MODE
	HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));
#endif
#if SPI_DMA_MODE
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command,
			sizeof(command));    //send re-use last payload
#endif
	csnHigh(psNRF24L01);
}
uint8_t getStatus(nRF24L01_struct_t *psNRF24L01) {
	return readReg(psNRF24L01, STATUS);
}

/* Payload */
uint8_t sendPayload(nRF24L01_struct_t *psNRF24L01, uint8_t *buf, size_t bufSize) {
	if (HAL_GPIO_ReadPin(psNRF24L01->hardware_struct.nRFportCSN,
			psNRF24L01->hardware_struct.nRFpinCSN)) {
		ceLow(psNRF24L01);
	}
	if (getStatusFullTxFIFO(psNRF24L01)) {
		flushTx(psNRF24L01);
	}
	if (getTX_DS(psNRF24L01)) {
		resetTX_DS(psNRF24L01);
	}
	if (writeTxPayload(psNRF24L01, buf, bufSize)) {
		ceHigh(psNRF24L01);
		delayUs(psNRF24L01, CE_HIGH_TIME);
		ceLow(psNRF24L01);
		if (getTX_DS(psNRF24L01)) {
			return OK_CODE;
		}
	}
	return 0;
}
uint8_t checkReceivedPayload(nRF24L01_struct_t *psNRF24L01, uint8_t pipe) {
	if (getPipeStatusRxFIFO(psNRF24L01) == pipe) {
		if (getRX_DR(psNRF24L01)) {
			resetRX_DR(psNRF24L01);
		}
		return 1;
	}
	return 0;
}

/**
 * @Setters and getters
 */

/* Power control */
void pwrUp(nRF24L01_struct_t *psNRF24L01) {
	uint8_t tmp = readReg(psNRF24L01, CONFIG);
	tmp |= (1 << 1);
	writeReg(psNRF24L01, CONFIG, tmp);
}
void pwrDown(nRF24L01_struct_t *psNRF24L01) {
	ceLow(psNRF24L01);
	uint8_t tmp = readReg(psNRF24L01, CONFIG);
	tmp &= (0 << 1);
	writeReg(psNRF24L01, CONFIG, tmp);
}

/* Switch to RX/TX mode or Standby-I */
void modeRX(nRF24L01_struct_t *psNRF24L01) {
	if (!readBit(psNRF24L01, CONFIG, bit1)) { //Check state of module
		pwrUp(psNRF24L01);
		delayUs(psNRF24L01, 1500); //wait 1.5ms fo nRF24L01+ stand up
	}
	flushRx(psNRF24L01); //clear (flush) RX FIFO buffer
	flushTx(psNRF24L01); //clear (flush) TX FIFO buffer

	resetRX_DR(psNRF24L01); //clear interrupts flags
	resetTX_DS(psNRF24L01);
	resetMAX_RT(psNRF24L01);
	//nRF in Standby-I
	ceHigh(psNRF24L01); //set high on CE line
	setBit(psNRF24L01, CONFIG, bit0);
	delayUs(psNRF24L01, RX_TX_SETTING_TIME);
}
void modeTX(nRF24L01_struct_t *psNRF24L01) {
	if (!readBit(psNRF24L01, CONFIG, bit1)) { //Check state of module
		pwrUp(psNRF24L01);
		delayUs(psNRF24L01, 1500); //wait 1.5ms fo nRF24L01+ stand up
	}
	flushRx(psNRF24L01); //clear (flush) RX FIFO buffer
	flushTx(psNRF24L01); //clear (flush) TX FIFO buffer

	resetRX_DR(psNRF24L01); //clear interrupts flags
	resetTX_DS(psNRF24L01);
	resetMAX_RT(psNRF24L01);

	ceHigh(psNRF24L01);
	resetBit(psNRF24L01, CONFIG, bit0);
	delayUs(psNRF24L01, RX_TX_SETTING_TIME);
}
void modeStandby(nRF24L01_struct_t *psNRF24L01) {
	ceLow(psNRF24L01);
	resetBit(psNRF24L01, CONFIG, bit0);
}

/* Interrupts functions */
void enableRXinterrupt(nRF24L01_struct_t *psNRF24L01) {
	setBit(psNRF24L01, CONFIG, bit6);
	psNRF24L01->settings_struct.enableRxIrq = 1;
}
void disableRXinterrupt(nRF24L01_struct_t *psNRF24L01) {
	resetBit(psNRF24L01, CONFIG, bit6); //disable RX_IRQ in Config register
	psNRF24L01->settings_struct.enableRxIrq = 0;
}
void enableTXinterrupt(nRF24L01_struct_t *psNRF24L01) {
	setBit(psNRF24L01, CONFIG, bit5);
	psNRF24L01->settings_struct.enableTxIrq = 1;
}
void disableTXinterrupt(nRF24L01_struct_t *psNRF24L01) {
	resetBit(psNRF24L01, CONFIG, bit5);
	psNRF24L01->settings_struct.enableTxIrq = 0;
}
void enableMaxRTinterrupt(nRF24L01_struct_t *psNRF24L01) {
	setBit(psNRF24L01, CONFIG, bit4);
	psNRF24L01->settings_struct.enableMaxRtIrq = 1;
}
void disableMaxRTinterrupt(nRF24L01_struct_t *psNRF24L01) {
	resetBit(psNRF24L01, CONFIG, bit4);
	psNRF24L01->settings_struct.enableMaxRtIrq = 0;
}

/* CRC functions */
void enableCRC(nRF24L01_struct_t *psNRF24L01) {
	setBit(psNRF24L01, CONFIG, bit3);
	psNRF24L01->settings_struct.enableCRC = 1;
}
void disableCRC(nRF24L01_struct_t *psNRF24L01) {
	resetBit(psNRF24L01, CONFIG, bit3);
	psNRF24L01->settings_struct.enableCRC = 0;
}
void setCRC(nRF24L01_struct_t *psNRF24L01, widthCRC_t w) {
	if (w) {
		setBit(psNRF24L01, CONFIG, bit2);
		psNRF24L01->settings_struct.codingCRC = 1;
	} else {
		resetBit(psNRF24L01, CONFIG, bit2);
		psNRF24L01->settings_struct.codingCRC = 0;
	}
}

/* Auto ACK */
uint8_t enableAutoAckPipe(nRF24L01_struct_t *psNRF24L01, uint8_t pipe) {
	if (checkPipe(pipe)) {
		setBit(psNRF24L01, EN_AA, pipe);
		psNRF24L01->settings_struct.pipeACK |= (1 << pipe);
		return 1;
	}
	return 0;
}
uint8_t disableAutoAckPipe(nRF24L01_struct_t *psNRF24L01, uint8_t pipe) {
	if (checkPipe(pipe)) {
		resetBit(psNRF24L01, EN_AA, pipe);
		psNRF24L01->settings_struct.pipeACK |= 0 << pipe;
		return 1;
	}
	return 0;
}

/* RX addresses */
uint8_t enableRxAddr(nRF24L01_struct_t *psNRF24L01, uint8_t pipe) {
	if (checkPipe(pipe)) {
		setBit(psNRF24L01, EN_RXADDR, pipe);
		psNRF24L01->settings_struct.pipeEn |= (1 << pipe);
		return 1;
	}
	return 0;
}
uint8_t disableRxAddr(nRF24L01_struct_t *psNRF24L01, uint8_t pipe) {
	if (checkPipe(pipe)) {
		resetBit(psNRF24L01, EN_RXADDR, pipe);
		psNRF24L01->settings_struct.pipeEn |= (0 << pipe);
		return 1;
	}
	return 0;
}

/* Address Width */
void setAddrWidth(nRF24L01_struct_t *psNRF24L01, addressWidth_t width) {
	writeReg(psNRF24L01, SETUP_AW, width);
	psNRF24L01->address_struct.addrWidth = width;
}

/* Setup retransmission */
uint8_t setAutoRetrCount(nRF24L01_struct_t *psNRF24L01, uint8_t count) {
	if (count >= 0x00 && count <= 0x0F) {                      //check count val
		uint8_t tmp = readReg(psNRF24L01, SETUP_RETR); //read reg. val
		tmp = tmp & 0xF0;                             // reset LSB and save MSB
		tmp |= count;                                 //add tmp and count
		writeReg(psNRF24L01, SETUP_RETR, tmp);         //write to SETUP_RETR
		psNRF24L01->settings_struct.ARC = count;
		return OK_CODE;
	}
	return ERR_CODE;
}
uint8_t setAutoRetrDelay(nRF24L01_struct_t *psNRF24L01, uint8_t delay) {
	if (delay > 0x0F) {                       //if delay in MSB format
		delay = delay >> 4; //shift to LSB format
	}
	if (delay >= 0x00 && delay <= 0x0F) {
		uint8_t tmp = readReg(psNRF24L01, SETUP_RETR);
		tmp = tmp & 0x0F;    //save LSB, reset MSB
		tmp |= (delay << 4); //add tmp and delay
		writeReg(psNRF24L01, SETUP_RETR, tmp);
		psNRF24L01->settings_struct.ARD = delay;
		return OK_CODE;
	}
	return ERR_CODE;
}

/* RF channel */
uint8_t setChannel(nRF24L01_struct_t *psNRF24L01, uint8_t channel) {
	if (channel >= 0 && channel <= 125) {
		writeReg(psNRF24L01, RF_CH, channel); //Maximum channel limited to 125 by hardware
		psNRF24L01->settings_struct.channel = channel;
		return OK_CODE;
	}
	return ERR_CODE;
}
/* RF setup */
void setRFpower(nRF24L01_struct_t *psNRF24L01, powerRF_t power) {
	if (power <= RF_PWR_0dBm && power >= RF_PWR_18dBm) {
		uint8_t tmp = readReg(psNRF24L01, RF_SETUP); //
		tmp = tmp & 0xF8;                        //0xF8 - 1111 1000B reset 3 LSB
		tmp = tmp | (power << 1);              //combining tmp and shifted power
		writeReg(psNRF24L01, RF_SETUP, tmp);
		psNRF24L01->settings_struct.powerRF = power;
	}
}
void setDataRate(nRF24L01_struct_t *psNRF24L01, dataRate_t rate) {
	uint8_t tmp = readReg(psNRF24L01, RF_SETUP); //
	tmp = tmp & 0x06; //0x06 = 0000 0110B - reset data rate's bits - Also this line reset PLL_LOCK and CONT_WAVE bits
	tmp = tmp | (rate << 3);               //combining tmp and shifted data rate
	writeReg(psNRF24L01, RF_SETUP, tmp);
	psNRF24L01->settings_struct.dataRate = rate;
}
#if ADVANCED_RF_OPT
void enableContCarrier(nRF24L01_struct_t *psNRF24L01)
{
	setBit(psNRF24L01, RF_SETUP, bit7);
}
void disableContCarrier(nRF24L01_struct_t *psNRF24L01)
{
	resetBit(psNRF24L01, RF_SETUP, bit7);
}
void enableLockPLL(nRF24L01_struct_t *psNRF24L01)
{
	setBit(psNRF24L01, RF_SETUP, bit4);
}
void diableLockPLL(nRF24L01_struct_t *psNRF24L01)
{
	resetBit(psNRF24L01, RF_SETUP, bit4);
}
#endif

/* Status register */
void resetRX_DR(nRF24L01_struct_t *psNRF24L01) { //clear irt bits in Status Register
	setBit(psNRF24L01, STATUS, bit6);
	psNRF24L01->status_struct.dataReadIrq = 0;
}
void resetTX_DS(nRF24L01_struct_t *psNRF24L01) {
	setBit(psNRF24L01, STATUS, bit5);
	psNRF24L01->status_struct.dataSendIrq = 0;
}
void resetMAX_RT(nRF24L01_struct_t *psNRF24L01) {
	setBit(psNRF24L01, STATUS, bit4);
	psNRF24L01->status_struct.maxRetr = 0;
}
void resetInterruptFlags(nRF24L01_struct_t *psNRF24L01) {
	writeReg(psNRF24L01, STATUS, 0x70);
	psNRF24L01->status_struct.dataReadIrq = 0;
	psNRF24L01->status_struct.dataSendIrq = 0;
	psNRF24L01->status_struct.maxRetr = 0;
}
uint8_t getRX_DR(nRF24L01_struct_t *psNRF24L01) {
	psNRF24L01->status_struct.dataReadIrq = readBit(psNRF24L01, STATUS, bit6);
	return (psNRF24L01->status_struct.dataReadIrq);
}
uint8_t getTX_DS(nRF24L01_struct_t *psNRF24L01) {
	psNRF24L01->status_struct.dataSendIrq = readBit(psNRF24L01, STATUS, bit5);
	return (psNRF24L01->status_struct.dataSendIrq);
}
uint8_t getMAX_RT(nRF24L01_struct_t *psNRF24L01) {
	psNRF24L01->status_struct.maxRetr = readBit(psNRF24L01, STATUS, bit4);
	return (psNRF24L01->status_struct.maxRetr);
}
uint8_t getInterruptFlags(nRF24L01_struct_t *psNRF24L01) {
	uint8_t tmp = readReg(psNRF24L01, STATUS);
	tmp = 4 >> (tmp & 0x70);
	if (tmp & 0x01)
		psNRF24L01->status_struct.maxRetr = 1;
	if (tmp & 0x02)
		psNRF24L01->status_struct.dataSendIrq = 1;
	if (tmp & 0x04)
		psNRF24L01->status_struct.dataReadIrq = 1;
	return tmp;
}

uint8_t getStatusFullTxFIFO(nRF24L01_struct_t *psNRF24L01) {
	/*
	 * 	if (readBit(psNRF24L01, STATUS, bit0)) {
	 psNRF24L01->status_struct.txFull = 1;
	 return 1; //TX FIFO full
	 }
	 psNRF24L01->status_struct.txFull = 0;
	 return 0; //Available locations in TX FIFO
	 */
	return (psNRF24L01->status_struct.txFull = readBit(psNRF24L01, STATUS, bit0));
}
uint8_t getPipeStatusRxFIFO(nRF24L01_struct_t *psNRF24L01) {
	/*
	 * //Zmieniono na kody bledow
	 uint8_t tmp = readReg(psNRF24L01, STATUS);
	 tmp &= 0x0E; //save only pipe number bits
	 tmp = tmp >> 1;
	 */
	uint8_t tmp = ((readReg(psNRF24L01, STATUS) & 0x0E) >> 1);
	if (checkPipe(tmp)) {
		psNRF24L01->status_struct.pipeNumber = tmp;
		return tmp;
	}
	if (tmp == 0x07) { //RX FIFO empty
		psNRF24L01->status_struct.pipeNumber = RX_FIFO_EMPTY;
		return RX_FIFO_EMPTY;
	}

	if (tmp == 0x06) { //110B - mean not used
		psNRF24L01->status_struct.pipeNumber = RX_FIFO_UNUSED;
		return RX_FIFO_UNUSED; //return ERR
	}
	return ERR_CODE;
}

/* Transmit observe */
uint8_t lostPacketsCount(nRF24L01_struct_t *psNRF24L01) {
	uint8_t tmp = readReg(psNRF24L01, OBSERVE_TX);
	tmp = (tmp >> 4);
	psNRF24L01->status_struct.packetsLost = tmp;
	return tmp;
}
uint8_t retrPacketsCount(nRF24L01_struct_t *psNRF24L01) {
	uint8_t tmp = readReg(psNRF24L01, OBSERVE_TX);
	tmp = (tmp & 0xF0);
	psNRF24L01->status_struct.packetsRetr = tmp;
	return tmp;
}
void clearlostPacketsCount(nRF24L01_struct_t *psNRF24L01) {
	uint8_t tmp = readReg(psNRF24L01, RF_CH);	//read RF_CH
	writeReg(psNRF24L01, RF_CH, tmp);			//clear by
}

#if ADVANCED_RF_OPT
/* RPD - for test use only */
uint8_t checkRPD(nRF24L01_struct_t *psNRF24L01)
{
	if (readReg(nrfStruct, RPD))
		return 1;
	else
		return 0;
}
#endif

/* RX pipes and TX address */
uint8_t setReceivePipeAddress(nRF24L01_struct_t *psNRF24L01, uint8_t pipe,
		uint8_t *addrBuf, size_t addrBufSize) {
	if (!checkPipe(pipe)) { //if checkPipe return 0 - end fun. by return 0.
		return ERR_CODE;
	}
	if (pipe == 0 || pipe == 1) {
		if (((psNRF24L01->address_struct.addrWidth) + 2) != addrBufSize) {
			return ERR_CODE;
		}
	} else {
		if (addrBufSize != 1) {
			return ERR_CODE;
		}
	}

	uint8_t addr = RX_ADDR_P0 + pipe;
	switch (pipe) { //check pipe and write addr to struct
	case 0:
		memcpy((void*) (psNRF24L01->address_struct.rxAddr0), (void*) addrBuf,
				addrBufSize);
		writeRegExt(psNRF24L01, addr, addrBuf, addrBufSize);
		break;
	case 1:
		memcpy((void*) (psNRF24L01->address_struct.rxAddr1), (void*) addrBuf,
				addrBufSize);
		writeRegExt(psNRF24L01, addr, addrBuf, addrBufSize);
		break;
	case 2:
		psNRF24L01->address_struct.rxAddr2 = *addrBuf;
		writeReg(psNRF24L01, addr, *addrBuf);
		break;
	case 3:
		psNRF24L01->address_struct.rxAddr3 = *addrBuf;
		writeReg(psNRF24L01, addr, *addrBuf);
		break;
	case 4:
		psNRF24L01->address_struct.rxAddr4 = *addrBuf;
		writeReg(psNRF24L01, addr, *addrBuf);
		break;
	case 5:
		psNRF24L01->address_struct.rxAddr5 = *addrBuf;
		writeReg(psNRF24L01, addr, *addrBuf);
		break;
	default:
		return ERR_CODE;
		break;
	}
	return OK_CODE;
}
uint8_t setTransmitPipeAddress(nRF24L01_struct_t *psNRF24L01, uint8_t *addrBuf,
		size_t addrBufSize) {
	if (((psNRF24L01->address_struct.addrWidth) + 2) != addrBufSize) {
		return ERR_CODE;
	}
	memcpy((void*) (psNRF24L01->address_struct.txAddr), (void*) addrBuf,
			addrBufSize);
	writeRegExt(psNRF24L01, TX_ADDR, addrBuf, addrBufSize);
	return OK_CODE;
}

/* RX Payload width */
uint8_t getRxPayloadWidth(nRF24L01_struct_t *psNRF24L01, uint8_t pipe) {
	if (checkPipe(pipe)) {
		uint8_t tmp = readReg(psNRF24L01, (RX_PW_P0 + pipe));
		psNRF24L01->settings_struct.pipePayLen[pipe] = tmp;
		return tmp;
	}
	return ERR_CODE;
}
uint8_t setRxPayloadWidth(nRF24L01_struct_t *psNRF24L01, uint8_t pipe,
		uint8_t width) {
	if (checkPipe(pipe)) {
		if (width < 1 && width > 32) { //check width correct value
			return ERR_CODE;
		}
		writeReg(psNRF24L01, (RX_PW_P0 + pipe), width);
		psNRF24L01->settings_struct.pipePayLen[pipe] = width;
		return OK_CODE;
	}
	return ERR_CODE;
}

/* FIFO status */
/* Return status of RX FIFO buffer by check bits in FIFO Status Register */
uint8_t getRxStatusFIFO(nRF24L01_struct_t *psNRF24L01) {
	uint8_t tmp = readReg(psNRF24L01, FIFO_STATUS);
	if ((tmp & 0x03) == RX_FIFO_MASK_EMPTY) {
		psNRF24L01->fifo_struct.rxEmpty = 1;
		psNRF24L01->fifo_struct.rxFull = 0;
		psNRF24L01->fifo_struct.rxRead = 0;
		return RX_FIFO_MASK_EMPTY; //RX FIFO register buffer is empty
	}
	if ((tmp & 0x03) == RX_FIFO_MASK_FULL) {
		psNRF24L01->fifo_struct.rxEmpty = 0;
		psNRF24L01->fifo_struct.rxFull = 1;
		psNRF24L01->fifo_struct.rxRead = 1;
		return RX_FIFO_MASK_FULL; ////RX FIFO register buffer is full
	}
	if ((tmp & 0x03) == RX_FIFO_MASK_DATA) {
		psNRF24L01->fifo_struct.rxEmpty = 0;
		psNRF24L01->fifo_struct.rxFull = 0;
		psNRF24L01->fifo_struct.rxRead = 1;
		return RX_FIFO_MASK_DATA; //RX FIFO register buffer has some data but isn't full
	}
	return ERR_CODE;
}
/* Return status of TX FIFO buffer by check bits in FIFO Status Register */
uint8_t getTxStatusFIFO(nRF24L01_struct_t *psNRF24L01) {
	uint8_t tmp = readReg(psNRF24L01, FIFO_STATUS);
	tmp = tmp >> 4;
	if ((tmp & 0x03) == TX_FIFO_MASK_EMPTY) {
		psNRF24L01->fifo_struct.txEmpty = 1;
		psNRF24L01->fifo_struct.txFull = 0;
		psNRF24L01->fifo_struct.txSend = 0;
		return TX_FIFO_MASK_EMPTY;
	}
	if ((tmp & 0x03) == TX_FIFO_MASK_FULL) {
		psNRF24L01->fifo_struct.txEmpty = 0;
		psNRF24L01->fifo_struct.txFull = 1;
		psNRF24L01->fifo_struct.txSend = 1;
		return TX_FIFO_MASK_FULL;
	}
	if ((tmp & 0x03) == TX_FIFO_MASK_DATA) {
		psNRF24L01->fifo_struct.txEmpty = 0;
		psNRF24L01->fifo_struct.txFull = 0;
		psNRF24L01->fifo_struct.txSend = 1;
		return TX_FIFO_MASK_DATA;
	}
	return ERR_CODE;
}

/* Checking reuse package */
uint8_t getTxReuse(nRF24L01_struct_t *psNRF24L01) {
	uint8_t tmp = readBit(psNRF24L01, FIFO_STATUS, TX_REUSE);
	psNRF24L01->fifo_struct.txReUse = tmp;
	if (tmp == 0x01) {
		return TX_REUSE_USED; /* TX_REUSE_USED mean that nRF24 module reuse to send again same package */
	}
	return TX_REUSE_UNUSED; /* TX_REUSE_UNUSED mena that nRF24 module doeasn't reuse to send again same package */
}

/* Dynamic Payload Length */
uint8_t enableDynamicPayloadLengthPipe(nRF24L01_struct_t *psNRF24L01,
		uint8_t pipe) {
	if (!checkPipe(pipe)) {
		return ERR_CODE;
	}
	setBit(psNRF24L01, DYNPD, pipe);
	psNRF24L01->settings_struct.pipeDPL |= (1 << pipe);
	return OK_CODE;
}
uint8_t disableDynamicPayloadLengthPipe(nRF24L01_struct_t *psNRF24L01,
		uint8_t pipe) {
	if (!checkPipe(pipe)) {
		return ERR_CODE;
	}
	resetBit(psNRF24L01, DYNPD, pipe);
	psNRF24L01->settings_struct.pipeDPL &= ~(1 << pipe);
	return OK_CODE;
}

/* Dynamic Payload Length */
void enableDynamicPayloadLength(nRF24L01_struct_t *psNRF24L01) {
	setBit(psNRF24L01, FEATURE, EN_DPL);
	psNRF24L01->settings_struct.enableDPL = 1;
}
void disableDynamicPayloadLength(nRF24L01_struct_t *psNRF24L01) {
	resetBit(psNRF24L01, FEATURE, EN_DPL);
	psNRF24L01->settings_struct.enableDPL = 0;
}

/* Payload with Auto Acknowladge */
void enableAckPayload(nRF24L01_struct_t *psNRF24L01) {
	setBit(psNRF24L01, FEATURE, EN_ACK_PAY);
	psNRF24L01->settings_struct.enableAckPay = 1;
}
void disableAckPayload(nRF24L01_struct_t *psNRF24L01) {
	resetBit(psNRF24L01, FEATURE, EN_ACK_PAY);
	psNRF24L01->settings_struct.enableAckPay = 0;
}

/* Enable W_TX_PAYLOAD_NOACK command */
void enableNoAckCommand(nRF24L01_struct_t *psNRF24L01) {
	setBit(psNRF24L01, FEATURE, EN_DYN_ACK);
}
void disableNoAckCommand(nRF24L01_struct_t *psNRF24L01) {
	resetBit(psNRF24L01, FEATURE, EN_DYN_ACK);
}

/**
 * @Static function for init structures
 */
static void nRF24L01_hardware_Init(nRF24L01_struct_t *psNRF24L01,
		SPI_HandleTypeDef *HAL_SPIx, TIM_HandleTypeDef *HAL_TIMx,
		GPIO_TypeDef *HAL_GPIO_CSN, uint16_t HAL_GPIO_Pin_CSN,
		GPIO_TypeDef *HAL_GPIO_CE, uint16_t HAL_GPIO_Pin_CE) {
	psNRF24L01->hardware_struct.nRFspi = HAL_SPIx;
	psNRF24L01->hardware_struct.nRFtim = HAL_TIMx;

	psNRF24L01->hardware_struct.nRFportCSN = HAL_GPIO_CSN;
	psNRF24L01->hardware_struct.nRFpinCSN = HAL_GPIO_Pin_CSN;

	psNRF24L01->hardware_struct.nRFportCE = HAL_GPIO_CE;
	psNRF24L01->hardware_struct.nRFpinCE = HAL_GPIO_Pin_CE;
}
static void nRF24L01_settings_Init(nRF24L01_struct_t *psNRF24L01) {
	/* Init settigns struct */
	psNRF24L01->settings_struct.rxMode = 0;                //set as receiver
	psNRF24L01->settings_struct.channel = 0x02;            //set channel np. 0
	psNRF24L01->settings_struct.dataRate = RF_DataRate_2M; //lowest data rate
	psNRF24L01->settings_struct.powerRF = RF_PWR_0dBm;     //-12dBm power

	psNRF24L01->settings_struct.ARD = 0; //auto retr. delay 250us
	psNRF24L01->settings_struct.ARC = 3; //auto retr. counter

	psNRF24L01->settings_struct.enableCRC = 1;
	psNRF24L01->settings_struct.codingCRC = 0;

	psNRF24L01->settings_struct.enableTxIrq = 0;
	psNRF24L01->settings_struct.enableRxIrq = 0;
	psNRF24L01->settings_struct.enableMaxRtIrq = 0;

	/* Pipe Enable - defult pipe 0 enable only */
	psNRF24L01->settings_struct.pipeEn = DF_RXADDR;
	psNRF24L01->settings_struct.pipeACK = DF_EN_AA;
	psNRF24L01->settings_struct.pipeDPL = DF_DYNPD;

	/* Pipe RX Payload Lenght  */
	uint8_t i;
	for (i = 0; i < 6; i++) {
		psNRF24L01->settings_struct.pipePayLen[i] = DF_RX_PW_P0;
	}

	psNRF24L01->settings_struct.enableDPL = 0;
	psNRF24L01->settings_struct.enableAckPay = 0;
	psNRF24L01->settings_struct.enableDynACK = 0; //enable NO_ACK command

}
static void nRF24L01_addresss_Init(nRF24L01_struct_t *psNRF24L01) {
	/* Init address struct */
	psNRF24L01->address_struct.addrWidth = longWidth;
	uint8_t i;
	for (i = 0; i < 5; i++) {
		psNRF24L01->address_struct.txAddr[i] = DF_TX_ADDR_0;
	}
	for (i = 0; i < 5; ++i) {
		psNRF24L01->address_struct.rxAddr0[i] = DF_RX_ADDR_P0_0;
	}
	for (i = 0; i < 5; ++i) {
		psNRF24L01->address_struct.rxAddr1[i] = DF_RX_ADDR_P1_0;
	}
	psNRF24L01->address_struct.rxAddr2 = DF_RX_ADDR_P2;
	psNRF24L01->address_struct.rxAddr3 = DF_RX_ADDR_P3;
	psNRF24L01->address_struct.rxAddr4 = DF_RX_ADDR_P4;
	psNRF24L01->address_struct.rxAddr5 = DF_RX_ADDR_P5;
}
static void nRF24L01_FIFO_Init(nRF24L01_struct_t *psNRF24L01) {
	/* Init fifo struct */
	psNRF24L01->fifo_struct.txReUse = 0;

	psNRF24L01->fifo_struct.rxRead = 0;
	psNRF24L01->fifo_struct.rxFull = 0;
	psNRF24L01->fifo_struct.rxEmpty = 1;

	psNRF24L01->fifo_struct.txSend = 0;
	psNRF24L01->fifo_struct.txFull = 0;
	psNRF24L01->fifo_struct.txEmpty = 1;

}
static void nRF24L01_status_Init(nRF24L01_struct_t *psNRF24L01) {

	psNRF24L01->status_struct.dataReadIrq = 0;
	psNRF24L01->status_struct.dataSendIrq = 0;
	psNRF24L01->status_struct.maxRetr = 0;
	psNRF24L01->status_struct.pipeNumber = RX_FIFO_EMPTY;
	psNRF24L01->status_struct.txFull = 0;

	psNRF24L01->status_struct.packetsLost = 0;
	psNRF24L01->status_struct.packetsRetr = 0;
}
static void nRF24L01_buffers_Init(nRF24L01_struct_t *psNRF24L01) {
	psNRF24L01->buffer_struct.ptBufferRX = psNRF24L01->buffer_struct.tBufferRX;
	psNRF24L01->buffer_struct.ptBufferTX = psNRF24L01->buffer_struct.tBufferTX;

	memset(psNRF24L01->buffer_struct.ptBufferRX, 0, RX_BUFFER_SIZE);
	memset(psNRF24L01->buffer_struct.ptBufferTX, 0, TX_BUFFER_SIZE);
}

/**
 * @
 */
/* Micro sencods delay - necessary to SPI transmittion in blocking mode and count time of CE states  */
void delayUs(nRF24L01_struct_t *psNRF24L01, uint16_t time) {
	__HAL_TIM_SET_COUNTER((psNRF24L01->hardware_struct.nRFtim), 0); //Set star value as 0
	while (__HAL_TIM_GET_COUNTER(psNRF24L01->hardware_struct.nRFtim) < time)
		; //
}

static uint8_t checkPipe(uint8_t pipe) {
	if (pipe >= 0 && pipe <= 5) //check correct pipe number
		return 1;
	return 0;
}

/* CE snd CSN control funtions's */
static void csnLow(nRF24L01_struct_t *psNRF24L01) {
	HAL_GPIO_WritePin((psNRF24L01->hardware_struct.nRFportCSN),
			(psNRF24L01->hardware_struct.nRFpinCSN), GPIO_PIN_RESET);
}
static void csnHigh(nRF24L01_struct_t *psNRF24L01) {
	HAL_GPIO_WritePin((psNRF24L01->hardware_struct.nRFportCSN),
			(psNRF24L01->hardware_struct.nRFpinCSN), GPIO_PIN_SET);
}
static void ceLow(nRF24L01_struct_t *psNRF24L01) {
	HAL_GPIO_WritePin((psNRF24L01->hardware_struct.nRFportCE),
			(psNRF24L01->hardware_struct.nRFpinCE), GPIO_PIN_RESET);
}
static void ceHigh(nRF24L01_struct_t *psNRF24L01) {
	HAL_GPIO_WritePin((psNRF24L01->hardware_struct.nRFportCE),
			(psNRF24L01->hardware_struct.nRFpinCE), GPIO_PIN_SET);
}

static uint8_t readBit(nRF24L01_struct_t *psNRF24L01, uint8_t addr,
		bitNum_t bit) {
	uint8_t reg = readReg(psNRF24L01, addr);
	return ((reg >> bit) & 0x01);
}
static void resetBit(nRF24L01_struct_t *psNRF24L01, uint8_t addr, bitNum_t bit) {
	uint8_t tmp = readReg(psNRF24L01, addr);
	tmp &= ~(1 << bit); //zmieniono OR na AND
	writeReg(psNRF24L01, addr, tmp);
}
static void setBit(nRF24L01_struct_t *psNRF24L01, uint8_t addr, bitNum_t bit) {
	uint8_t tmp = readReg(psNRF24L01, addr);
	tmp |= (1 << bit);
	writeReg(psNRF24L01, addr, tmp);
}
