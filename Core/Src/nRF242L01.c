/*
 * nRF242L01.c
 *
 *  Created on: 1 lis 2020
 *      Author: Khevenin
 */
#include "main.h"
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
 * @Main init function
 */
bool nRF_Init(nRF24L01_struct_t *psNRF24L01, SPI_HandleTypeDef *HAL_SPIx, TIM_HandleTypeDef *HAL_TIMx,
		GPIO_TypeDef *HAL_GPIO_CSN,
		uint16_t HAL_GPIO_Pin_CSN, GPIO_TypeDef *HAL_GPIO_CE, uint16_t HAL_GPIO_Pin_CE) {
	if (psNRF24L01 == NULL || HAL_SPIx == NULL || HAL_TIMx == NULL || HAL_GPIO_CSN == NULL || HAL_GPIO_CE == NULL) {
		return false;
	}
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
	psNRF24L01->settings_struct.setStruct.rxMode = 0;                //set as receiver
	psNRF24L01->settings_struct.setStruct.channel = 0x02;            //set channel np. 0
	psNRF24L01->settings_struct.setStruct.dataRate = RF_DataRate_2M; //lowest data rate
	psNRF24L01->settings_struct.setStruct.powerRF = RF_PWR_0dBm;     //-12dBm power

	psNRF24L01->settings_struct.setStruct.ard = 0; //auto retr. delay 250us
	psNRF24L01->settings_struct.setStruct.arc = 3; //auto retr. counter

	psNRF24L01->settings_struct.setStruct.enableCRC = 1;
	psNRF24L01->settings_struct.setStruct.codingCRC = 0;

	psNRF24L01->settings_struct.setStruct.enableTxIrq = 0;
	psNRF24L01->settings_struct.setStruct.enableRxIrq = 0;
	psNRF24L01->settings_struct.setStruct.enableMaxRtIrq = 0;

	/* Pipe Enable - defult pipe 0 enable only */
	psNRF24L01->settings_struct.setStruct.pipeEn = DF_RXADDR;
	psNRF24L01->settings_struct.setStruct.pipeACK = DF_EN_AA;
	psNRF24L01->settings_struct.setStruct.pipeDPL = DF_DYNPD;

	/* Pipe RX Payload Lenght  */
	uint8_t i;
	for (i = 0; i < 6; i++) {
		psNRF24L01->settings_struct.setStruct.pipePayLen[i] = DF_RX_PW_P0;
	}

	psNRF24L01->settings_struct.setStruct.enableDPL = 0;
	psNRF24L01->settings_struct.setStruct.enableAckPay = 0;
	psNRF24L01->settings_struct.setStruct.enableDynACK = 0; //enable NO_ACK command

}
