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
void delayUs(nRF24L01_struct_t *psNRF24L01, uint16_t time);
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

/* Elementary functions  nRf24L01+  */
/* Read and write registers funtions's */
uint8_t readReg(nRF24L01_struct_t *psNRF24L01, uint8_t addr) {
	uint8_t command = R_REGISTER | addr;
	uint8_t data;

	csnLow(psNRF24L01);
	/*
	 HAL_SPI_Transmit((psNRF24L01->hardware_struct.nRFspi), &command, sizeof(command), SPI_TIMEOUT);
	 HAL_Delay(1);
	 HAL_SPI_Receive((psNRF24L01->hardware_struct.nRFspi), &data, 1, SPI_TIMEOUT);

	 HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));
	 HAL_SPI_Receive_IT(psNRF24L01->hardware_struct.nRFspi, &data, sizeof(data));
	 */
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));
	HAL_SPI_Receive_DMA(psNRF24L01->hardware_struct.nRFspi, &data, 1);

	csnHigh(psNRF24L01);

	return data;
}
void writeReg(nRF24L01_struct_t *psNRF24L01, uint8_t addr, uint8_t val) {
	uint8_t command = W_REGISTER | addr;
	uint8_t data = val;

	csnLow(psNRF24L01);
	/*
	 HAL_SPI_Transmit((psNRF24L01->hardware_struct.nRFspi), &command, sizeof(command), SPI_TIMEOUT);
	 HAL_Delay(1);
	 HAL_SPI_Transmit((psNRF24L01->hardware_struct.nRFspi), &data, sizeof(data), SPI_TIMEOUT);

	 HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));
	 HAL_SPI_Transmit_IT(psNRF24L01->hardware_struct.nRFspi, &data, sizeof(data));
	 */
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &data, sizeof(data));

	csnHigh(psNRF24L01);
}

/*
 * @Extended read and write functions - R/W few registers
 */
void readRegExt(nRF24L01_struct_t *psNRF24L01, uint8_t addr, uint8_t *pBuf, size_t bufSize) {
	if (psNRF24L01 != NULL && pBuf != NULL && bufSize > 0) {
		uint8_t command = 0;
		uint8_t i = 0;

		csnLow(psNRF24L01);
		do {
			command = R_REGISTER | (addr + i);	//set command
			HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));	//transmit command
			HAL_SPI_Receive_DMA(psNRF24L01->hardware_struct.nRFspi, (pBuf + i), sizeof(uint8_t));	//receive data
			i++;
		} while (i != bufSize);
		csnHigh(psNRF24L01);
	}
}
void writeRegExt(nRF24L01_struct_t *psNRF24L01, uint8_t addr, uint8_t *pBuf, size_t bufSize) {
	if (psNRF24L01 != NULL && pBuf != NULL && bufSize > 0) {
		uint8_t command = 0;
		uint8_t i = 0;

		csnLow(psNRF24L01);
		do {
			command = W_REGISTER | (addr + i);	//set command
			HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));	//transmit command
			HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, (pBuf + i), sizeof(uint8_t));	//transmit data
			i++;
		} while (i != bufSize);
		csnHigh(psNRF24L01);
	}
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
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command)); //Flush TX
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
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command)); //Flush RX
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
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));    //send re-use last payload
	csnHigh(psNRF24L01);
}
uint8_t getStatus(nRF24L01_struct_t *psNRF24L01) {
	uint8_t command = NOP;
	uint8_t reg = 0;

	csnLow(psNRF24L01);
	HAL_SPI_Transmit_DMA(psNRF24L01->hardware_struct.nRFspi, &command, sizeof(command));    //send get command
	HAL_SPI_Receive_DMA(psNRF24L01->hardware_struct.nRFspi, &reg, sizeof(reg)); 			//get status
	csnHigh(psNRF24L01);

	return reg;
}

/* Payload */
uint8_t sendPayload(nRF24L01_struct_t *psNRF24L01, uint8_t *buf, size_t bufSize) {
	if (HAL_GPIO_ReadPin(psNRF24L01->hardware_struct.nRFportCSN, psNRF24L01->hardware_struct.nRFpinCSN)) {
		ceLow(psNRF24L01);
	}
	if (getStatusFullTxFIFO(psNRF24L01)) {
		flushTx(psNRF24L01);
	}
	if (getTX_DS(psNRF24L01)) {
		clearTX_DS(psNRF24L01);
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
			clearRX_DR(psNRF24L01);
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

/* Switch to RX/TX mode or Standby */
void modeRX(nRF24L01_struct_t *psNRF24L01) {
	if (!readBit(psNRF24L01, CONFIG, bit1)) { //Check state of module
		pwrUp(psNRF24L01);
		delayUs(psNRF24L01, 1500); //wait 1.5ms fo nRF24L01+ stand up
	}
	flushRx(psNRF24L01); //clear (flush) RX FIFO buffer
	flushTx(psNRF24L01); //clear (flush) TX FIFO buffer

	clearRX_DR(psNRF24L01); //clear interrupts flags
	clearTX_DS(psNRF24L01);
	clearMAX_RT(psNRF24L01);
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

	clearRX_DR(psNRF24L01); //clear interrupts flags
	clearTX_DS(psNRF24L01);
	clearMAX_RT(psNRF24L01);

	ceHigh(psNRF24L01);
	resetBit(psNRF24L01, CONFIG, bit0);
	delayUs(psNRF24L01, RX_TX_SETTING_TIME);
}
void modeStandby(nRF24L01_struct_t *psNRF24L01) {
	ceLow(psNRF24L01);
	resetBit(psNRF24L01, CONFIG, bit0);
}





/* Transmit address data pipe */
uint8_t setTransmitPipeAddress(nRF24L01_struct_t *psNRF24L01, uint8_t *addrBuf, size_t addrBufSize) {
	if (((psNRF24L01->address_struct.addrWidth) + 2) != addrBufSize) {
		return ERR_CODE;
	}

	uint8_t i;
	for (i = 0; i < addrBufSize; i++) { //write to struct
		psNRF24L01->address_struct.txAddr[i] = addrBuf[i];
	}
	writeRegExt(psNRF24L01, TX_ADDR, addrBuf, addrBufSize);
	return OK_CODE;
}

uint8_t setRxPayloadWidth(nRF24L01_struct_t *psNRF24L01, uint8_t pipe, uint8_t width) {
	if (checkPipe(pipe)) {
		if (width < 1 && width > 32) { //check width correct value
			return ERR_CODE;
		}
		uint8_t addr = RX_PW_P0 + pipe;
		writeReg(psNRF24L01, addr, width);
		psNRF24L01->settings_struct.pipePayLen[pipe] = width;
		return OK_CODE;
	}
	return ERR_CODE;
}

/* RX Payload width */
uint8_t getRxPayloadWidth(nRF24L01_struct_t *psNRF24L01, uint8_t pipe) {
	if (checkPipe(pipe)) {
		uint8_t addr = RX_PW_P0 + pipe;
		uint8_t tmp = readReg(psNRF24L01, addr);
		psNRF24L01->settings_struct.pipePayLen[pipe] = tmp;
		return tmp;
	}
	return ERR_CODE;
}

/* FIFO status */
/**
 * @Brief	Return status of RX FIFO buffer by check bits in FIFO Status Register
 * */
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
/**
 * @Brief	Return status of TX FIFO buffer by check bits in FIFO Status Register
 * */
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
/**
 * @Brief	Checking reuse package
 * @Retval	TX_REUSE_USED mean that nRF24 module reuse to send again same package
 * 			TX_REUSE_UNUSED mena that nRF24 module doeasn't reuse to send again same package
 **/
uint8_t getTxReuse(nRF24L01_struct_t *psNRF24L01) {
	uint8_t tmp = readBit(psNRF24L01, FIFO_STATUS, TX_REUSE);
	psNRF24L01->fifo_struct.txReUse = tmp;
	if (tmp == 0x01) {
		return TX_REUSE_USED;
	}
	return TX_REUSE_UNUSED;
}

/* Dynamic Payload Lenggth */
uint8_t enableDynamicPayloadLengthPipe(nRF24L01_struct_t *psNRF24L01, uint8_t pipe) {
	if (!checkPipe(pipe)) {
		return ERR_CODE;
	}
	setBit(psNRF24L01, DYNPD, pipe);
	psNRF24L01->settings_struct.pipeDPL |= (1 << pipe);
	return OK_CODE;
}

uint8_t disableDynamicPayloadLengthPipe(nRF24L01_struct_t *psNRF24L01, uint8_t pipe) {
	if (!checkPipe(pipe)) {
		return ERR_CODE;
	}
	resetBit(psNRF24L01, DYNPD, pipe);
	psNRF24L01->settings_struct.pipeDPL |= (0 << pipe);
	return OK_CODE;
}
/* Feature */
void enableDynamicPayloadLength(nRF24L01_struct_t *psNRF24L01) {
	setBit(psNRF24L01, FEATURE, EN_DPL);
	psNRF24L01->settings_struct.enableDPL = 1;
}
void disableDynamicPayloadLength(nRF24L01_struct_t *psNRF24L01) {
	resetBit(psNRF24L01, FEATURE, EN_DPL);
	psNRF24L01->settings_struct.enableDPL = 0;
}

void enableAckPayload(nRF24L01_struct_t *psNRF24L01) {
	setBit(psNRF24L01, FEATURE, EN_ACK_PAY);
	psNRF24L01->settings_struct.enableAckPay = 1;
}
void disableAckPayload(nRF24L01_struct_t *psNRF24L01) {
	resetBit(psNRF24L01, FEATURE, EN_ACK_PAY);
	psNRF24L01->settings_struct.enableAckPay = 0;
}

/**
 * @Brief	Enable W_TX_PAYLOAD_NOACK command
 * */
void enableNoAckCommand(nRF24L01_struct_t *psNRF24L01) {
	setBit(psNRF24L01, FEATURE, EN_DYN_ACK);
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
/* Micro sencods delay - necessary to SPI transmittion in blocking mode and count time of CE states  */
void delayUs(nRF24L01_struct_t *psNRF24L01, uint16_t time) {
__HAL_TIM_SET_COUNTER((psNRF24L01->hardware_struct.nRFtim), 0); //Set star value as 0
while (__HAL_TIM_GET_COUNTER(psNRF24L01->hardware_struct.nRFtim) < time)
	; //
}

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


























