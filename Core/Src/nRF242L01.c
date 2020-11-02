/*
 * nRF242L01.c
 *
 *  Created on: 1 lis 2020
 *      Author: Khevenin
 */

#include "nRF24L01.h"

/**
 * @Static function for init structures
 */
static void nRF24L01_hardware_Init(nRF24L01_struct_t *psNRF24L01, SPI_HandleTypeDef *HAL_SPIx,
		TIM_HandleTypeDef *HAL_TIMx, GPIO_TypeDef *HAL_GPIO_CSN, uint16_t HAL_GPIO_Pin_CSN, GPIO_TypeDef *HAL_GPIO_CE,
		uint16_t HAL_GPIO_Pin_CE);
static void nRF24L01_settings_Init(nRF24L01_struct_t *psNRF24L01);
static void nRF24L01_addresss_Init(nRF24L01_struct_t *psNRF24L01);
static void nRF24L01_FIFO_Init(nRF24L01_struct_t *psNRF24L01);
static void nRF24L01_status_Init(nRF24L01_struct_t *psNRF24L01);
static void nRF24L01_buffers_Init(nRF24L01_struct_t *psNRF24L01);

/**

 * @
 */
#if SPI_BLOCKING_MODE
static void delayUs(nRF24L01_struct_t *psNRF24L01, uint16_t time);
#endif
static void csnLow(nRF24L01_struct_t *psNRF24L01);
static void csnHigh(nRF24L01_struct_t *psNRF24L01);
static void ceLow(nRF24L01_struct_t *psNRF24L01);
static void ceHigh(nRF24L01_struct_t *psNRF24L01);

static uint8_t readBit(nRF24L01_struct_t *psNRF24L01, uint8_t addr, bitNum_t bit);
static void resetBit(nRF24L01_struct_t *psNRF24L01, uint8_t addr, bitNum_t bit);
static void setBit(nRF24L01_struct_t *psNRF24L01, uint8_t addr, bitNum_t bit);


/**
 * @Main init function
 */
bool nRF_Init(nRF24L01_struct_t *psNRF24L01, SPI_HandleTypeDef *HAL_SPIx, TIM_HandleTypeDef *HAL_TIMx,
		GPIO_TypeDef *HAL_GPIO_CSN,
		uint16_t HAL_GPIO_Pin_CSN, GPIO_TypeDef *HAL_GPIO_CE, uint16_t HAL_GPIO_Pin_CE) {
	if (psNRF24L01 == NULL || HAL_SPIx == NULL || HAL_TIMx == NULL || HAL_GPIO_CSN == NULL || HAL_GPIO_CE == NULL) {
		return false;
	}

	nRF24L01_hardware_Init(psNRF24L01, HAL_SPIx, HAL_TIMx, HAL_GPIO_CSN, HAL_GPIO_Pin_CSN, HAL_GPIO_CE,
			HAL_GPIO_Pin_CE);
	nRF24L01_settings_Init(psNRF24L01);
	nRF24L01_addresss_Init(psNRF24L01);
	nRF24L01_FIFO_Init(psNRF24L01);
	nRF24L01_status_Init(psNRF24L01);
	nRF24L01_buffers_Init(psNRF24L01);

	return true;
}


/**
 * @Static function for init structures
 */
static void nRF24L01_hardware_Init(nRF24L01_struct_t *psNRF24L01, SPI_HandleTypeDef *HAL_SPIx,
		TIM_HandleTypeDef *HAL_TIMx, GPIO_TypeDef *HAL_GPIO_CSN, uint16_t HAL_GPIO_Pin_CSN, GPIO_TypeDef *HAL_GPIO_CE,
		uint16_t HAL_GPIO_Pin_CE) {
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
/* Micro sencods delay - necessary to SPI transmittion  */
#if SPI_BLOCKING_MODE
static void delayUs(nRF24L01_struct_t *psNRF24L01, uint16_t time) {

	__HAL_TIM_SET_COUNTER((psNRF24L01->hardware_struct.nRFtim), 0); //Set star value as 0
	while (__HAL_TIM_GET_COUNTER(psNRF24L01->hardware_struct.nRFtim) < time); //
}
#endif
/* CE snd CSN control funtions's */
static void csnLow(nRF24L01_struct_t *psNRF24L01) {
	HAL_GPIO_WritePin((psNRF24L01->hardware_struct.nRFportCSN), (psNRF24L01->hardware_struct.nRFpinCSN),
			GPIO_PIN_RESET);
}
static void csnHigh(nRF24L01_struct_t *psNRF24L01) {
	HAL_GPIO_WritePin((psNRF24L01->hardware_struct.nRFportCSN), (psNRF24L01->hardware_struct.nRFpinCSN), GPIO_PIN_SET);
}
static void ceLow(nRF24L01_struct_t *psNRF24L01) {
	HAL_GPIO_WritePin((psNRF24L01->hardware_struct.nRFportCE), (psNRF24L01->hardware_struct.nRFpinCE), GPIO_PIN_RESET);
}
static void ceHigh(nRF24L01_struct_t *psNRF24L01) {
	HAL_GPIO_WritePin((psNRF24L01->hardware_struct.nRFportCE), (psNRF24L01->hardware_struct.nRFpinCE), GPIO_PIN_SET);
}

static uint8_t readBit(nRF24L01_struct_t *psNRF24L01, uint8_t addr, bitNum_t bit) {
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


/* Elementary functions  nRf24L01+  */
/* Read and write registers funtions's */
uint8_t readReg(nRF24L01_struct_t *psNRF24L01, uint8_t addr) {
	uint8_t command = R_REGISTER | addr;
	uint8_t data;

	csnLow(psNRF24L01);
#if SPI_BLOCKING_MODO
	HAL_SPI_Transmit((psNRF24L01->hardware_struct.nRFspi), pCmd, sizeof(cmd), SPI_TIMEOUT);
	delayUs(psNRF24L01, 50);
	HAL_SPI_Receive((psNRF24L01->hardware_struct.nRFspi), pReg, sizeof(reg), SPI_TIMEOUT);
#endif
#if SPI_DMA_MODE
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));
	HAL_SPI_Receive_DMA(psNRF24L01->hardware_struct.nRFspi, &data, sizeof(data));
#endif
	csnHigh(psNRF24L01);
	return data;
}
void writeReg(nRF24L01_struct_t *psNRF24L01, uint8_t addr, uint8_t val) {
	uint8_t command = W_REGISTER | addr;

	csnLow(psNRF24L01);

#if SPI_BLOCKING_MODO
    HAL_SPI_Transmit((psNRF24L01->nRFspi), pCmd, sizeof(cmd), SPI_TIMEOUT);
    delayUs(psNRF24L01, 50);
    HAL_SPI_Transmit((psNRF24L01->nRFspi), &val, sizeof(val), SPI_TIMEOUT);
#endif
#if SPI_DMA_MODE
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &val, sizeof(val));
#endif
	csnHigh(psNRF24L01);
}
/* Extended read and write functions - R/W few registers */
void readRegExt(nRF24L01_struct_t *psNRF24L01, uint8_t addr, uint8_t *pBuf, size_t bufSize) {
	uint8_t command = 0;

	csnLow(psNRF24L01);
#if SPI_BLOCKING_MODO
	HAL_SPI_Transmit((psNRF24L01->nRFspi), pCmd, sizeof(cmd), SPI_TIMEOUT);
	delayUs(psNRF24L01, 50);
	HAL_SPI_Receive((psNRF24L01->nRFspi), buf, bufSize,
	SPI_TIMEOUT);
#endif
#if SPI_DMA_MODE
	uint8_t i = 0;
	do {
		command = R_REGISTER | (addr + i);
		HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));
		HAL_SPI_Receive_DMA(psNRF24L01->hardware_struct.nRFspi, (pBuf + i), sizeof(unsigned short int));
		i++;
	} while (i != bufSize);

#endif
	csnHigh(psNRF24L01);
}
void writeRegExt(nRF24L01_struct_t *psNRF24L01, uint8_t addr, uint8_t *pBuf, size_t bufSize) {
	uint8_t command = 0;

	csnLow(psNRF24L01);
#if SPI_BLOCKING_MODO
	HAL_SPI_Transmit((psNRF24L01->nRFspi), pCmd, sizeof(cmd), SPI_TIMEOUT);
	delayUs(psNRF24L01, 50);
	HAL_SPI_Transmit((psNRF24L01->nRFspi), buf, bufSize, SPI_TIMEOUT);
#endif
#if SPI_DMA_MODE
	uint8_t i = 0;
	do {
		command = R_REGISTER | (addr + i);
		HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));
		HAL_SPI_Receive_DMA(psNRF24L01->hardware_struct.nRFspi, (pBuf + i), sizeof(unsigned short int));
		i++;
	} while (i != bufSize);
#endif
	csnHigh(psNRF24L01);
}

/* Payload's functions */
uint8_t readRxPayload(nRF24L01_struct_t *psNRF24L01, uint8_t *pBuf, size_t bufSize) {
	if (bufSize < 1)
		return ERR_CODE;
	if (bufSize > 32)
		bufSize = 32;

	uint8_t command = R_RX_PAYLOAD; //set command mask

	csnLow(psNRF24L01);
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));	//send command
	HAL_SPI_Receive_DMA(psNRF24L01->hardware_struct.nRFspi, pBuf, bufSize); 				//read payload
	csnHigh(psNRF24L01);

	return OK_CODE;
}
uint8_t writeTxPayload(nRF24L01_struct_t *psNRF24L01, uint8_t *pBuf, size_t bufSize) {
	if (bufSize < 1)
		return ERR_CODE;
	if (bufSize > 32)
		bufSize = 32;

	uint8_t command = W_TX_PAYLOAD; //set command mask

	csnLow(psNRF24L01);
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command)); //send command
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, pBuf, bufSize);	//write payload
	csnHigh(psNRF24L01);

	return OK_CODE;
}
uint8_t writeTxPayloadAck(nRF24L01_struct_t *psNRF24L01, uint8_t *pBuf, size_t bufSize) {
	if (bufSize < 1)
		return ERR_CODE;
	if (bufSize > 32)
		bufSize = 32;

	uint8_t command = W_ACK_PAYLOAD; //set command mask

	csnLow(psNRF24L01);
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command)); 	//send command
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, pBuf, bufSize);				//write payload
	csnHigh(psNRF24L01);

	return OK_CODE;
}
uint8_t writeTxPayloadNoAck(nRF24L01_struct_t *psNRF24L01, uint8_t *pBuf, size_t bufSize) {
	if (bufSize < 1)
		return ERR_CODE;
	if (bufSize > 32)
		bufSize = 32;

	uint8_t command = W_TX_PAYLOAD_NO_ACK; //set command mask

	csnLow(psNRF24L01);
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));    //send command
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, pBuf, bufSize);		   		//write payload
	csnHigh(psNRF24L01);

	return OK_CODE;
}

/* Read payload lenght when DPL enable */
uint8_t readDynamicPayloadWidth(nRF24L01_struct_t *psNRF24L01) {
	uint8_t command = R_RX_PL_WID; //set command mask
	uint8_t width;

	csnLow(psNRF24L01);
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command)); //send command
	HAL_SPI_Receive_DMA(psNRF24L01->hardware_struct.nRFspi, &width, sizeof(width)); //read payload width
	csnHigh(psNRF24L01);
	
	return width;
}

/* Flush functions */
uint8_t flushTx(nRF24L01_struct_t *psNRF24L01) {
	uint8_t command = FLUSH_TX; //set command mask

	csnLow(psNRF24L01);
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command)); //send command
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
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command)); //send command
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
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));    //send command
	csnHigh(psNRF24L01);
}
uint8_t getStatus(nRF24L01_struct_t *psNRF24L01) {
	uint8_t command = NOP;
	uint8_t reg = 0;


	csnLow(psNRF24L01);
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));    //send command
	HAL_SPI_Receive_DMA(psNRF24L01->hardware_struct.nRFspi, &reg, sizeof(reg)); //read payload width
	csnHigh(psNRF24L01);

	return reg;
}























