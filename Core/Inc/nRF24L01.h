/*
 * nRF24L01.h
 *
 *  Created on: 1 lis 2020
 *      Author: Khevenin
 */

#ifndef INC_NRF24L01_H_
#define INC_NRF24L01_H_

/**
 * Includes
 */
#include <stdbool.h>
#include "main.h"
#include "string.h"


/**
 * Map rregisters
 **/

/** Control registers */
#define CONFIG 0x00     //PWR, role, interrupts
#define EN_AA 0x01      //enable auto ack
#define EN_RXADDR 0x02  //enable RX addresses
#define SETUP_AW 0x03   //setup of address widths
#define SETUP_RETR 0x04 //setup of automatic retrasmission
#define RF_CH 0x05      //RF channel
#define RF_SETUP 0x06   //RF setup register
#define STATUS 0x07     //Status register
#define OBSERVE_TX 0x08 //Transmit observe register
#define RPD 0x09        //Received Power Detector

/** Address registers */
#define RX_ADDR_P0 0x0A //Received address data pipe 0
#define RX_ADDR_P1 0x0B //Received address data pipe 1
#define RX_ADDR_P2 0x0C //Received address data pipe 2
#define RX_ADDR_P3 0x0D //Received address data pipe 3
#define RX_ADDR_P4 0x0E //Received address data pipe 4
#define RX_ADDR_P5 0x0F //Received address data pipe 5

#define TX_ADDR 0x10 //Transmit addresses

/** Size of payload */
#define RX_PW_P0 0x11 //Number of bytes in RX payload in data pipe 0 - max 32 bytes
#define RX_PW_P1 0x12 //Number of bytes in RX payload in data pipe 1 - max 32 bytes
#define RX_PW_P2 0x13 //Number of bytes in RX payload in data pipe 2 - max 32 bytes
#define RX_PW_P3 0x14 //Number of bytes in RX payload in data pipe 3 - max 32 bytes
#define RX_PW_P4 0x15 //Number of bytes in RX payload in data pipe 4 - max 32 bytes
#define RX_PW_P5 0x16 //Number of bytes in RX payload in data pipe 5 - max 32 bytes

/** Another feature */
#define FIFO_STATUS 0x17 //FIFO status register
#define DYNPD 0x1C       //Enable dynamic payload lenght
#define FEATURE 0x1D     // Feature register

/**
 * Deafult values of registers
 **/
#define DF_CONFIG 0x08
#define DF_EN_AA 0x3F //DEX - 63
#define DF_RXADDR 0x03
#define DF_SETUP_AW 0x03
#define DF_SETUP_RETR 0x03
#define DF_RF_CH 0x02
#define DF_RF_SETUP 0x0E
#define DF_STATUS 0x0E     // B - 3:0 - read only
#define DF_OBSERVE_TX 0x00 //read only
#define DF_RPD 0x00        //read only

#define DF_RX_ADDR_P0_0 0xE7
#define DF_RX_ADDR_P0_1 0xE7
#define DF_RX_ADDR_P0_2 0xE7
#define DF_RX_ADDR_P0_3 0xE7
#define DF_RX_ADDR_P0_4 0xE7

#define DF_RX_ADDR_P1_0 0xC2
#define DF_RX_ADDR_P1_1 0xC2
#define DF_RX_ADDR_P1_2 0xC2
#define DF_RX_ADDR_P1_3 0xC2
#define DF_RX_ADDR_P1_4 0xC2

#define DF_RX_ADDR_P2 0xC3
#define DF_RX_ADDR_P3 0xC4
#define DF_RX_ADDR_P4 0xC5
#define DF_RX_ADDR_P5 0xC6

#define DF_TX_ADDR_0 0xE7
#define DF_TX_ADDR_1 0xE7
#define DF_TX_ADDR_2 0xE7
#define DF_TX_ADDR_3 0xE7
#define DF_TX_ADDR_4 0xE7

#define DF_RX_PW_P0 0x00
#define DF_RX_PW_P1 0x00
#define DF_RX_PW_P2 0x00
#define DF_RX_PW_P3 0x00
#define DF_RX_PW_P4 0x00
#define DF_RX_PW_P5 0x00

#define DF_FIFO_STATUS 0x11 // B - 6:4, 1:0 - read only
#define DF_DYNPD 0x00
#define DF_FEATURE 0x00

/* Config register */
#define MASK_RX_DR bit6
#define MASK_TX_DS bit5
#define MASK_MAX_RT bit4
#define EN_CRC bit3
#define CRCO bit2
#define PWR_UP bit1
#define PRIM_RX bit0

/* RF setup register */
#define CONT_WAVE bit7
#define RF_DR_LOW bit5
#define PLL_LOCK bit4
#define RF_DR_HIGH bit3
#define RF_PWR_HIGH bit2
#define RF_PWR_LOW bit1

/* Status register */
#define RX_DR bit6  //Data read interrupt
#define TX_DS bit5  //Data send interrupt
#define MAX_RT bit4 //Max number of retransmits interrupt
#define TX_FULL bit0

/* Received Power Detector register */
#define RPD_FLAG bit0

/* FIFO status register */
#define TX_REUSE bit6
#define TX_FULL_FIFO bit5
#define TX_EMPTY bit4
#define RX_FULL bit1
#define RX_EMPTY bit0

/* Dynamic Payload register */
#define DPL_P5 bit5
#define DPL_P4 bit4
#define DPL_P3 bit3
#define DPL_P2 bit2
#define DPL_P1 bit1
#define DPL_P0 bit0

/* Feature register */
#define EN_DPL bit2
#define EN_ACK_PAY bit1
#define EN_DYN_ACK bit0

/**
 * nRF24L01+ internal commands
 */
#define R_REGISTER 0x00 //000a aaaa
#define W_REGISTER 0x20 //001a aaaa
#define R_RX_PAYLOAD 0x61
#define W_TX_PAYLOAD 0xA0
#define FLUSH_TX 0xE1
#define FLUSH_RX 0xE0
#define REUSE_TX_PL 0xE0
#define R_RX_PL_WID 0x60
#define W_ACK_PAYLOAD 0xA8 //1010 1ppp
#define W_TX_PAYLOAD_NO_ACK 0xB0
#define NOP 0xFF

/*
 * Functions return codes
 */
#define OK_CODE 0x01
#define ERR_CODE 0xFF

#define RX_FIFO_EMPTY 0x07
#define RX_FIFO_UNUSED 0x06

#define RX_FIFO_MASK_DATA 0x00
#define RX_FIFO_MASK_EMPTY 0x01
#define RX_FIFO_MASK_FULL 0x02

#define TX_FIFO_MASK_DATA 0x00
#define TX_FIFO_MASK_EMPTY 0x01
#define TX_FIFO_MASK_FULL 0x02

#define TX_REUSE_UNUSED 0x00
#define TX_REUSE_USED 0x01

#define SPI_TIMEOUT 10
#define CE_HIGH_TIME 11
#define RX_TX_SETTING_TIME 140

/**
 * @TX and RX buffers sizes
 */
#define TX_BUFFER_SIZE 32
#define RX_BUFFER_SIZE 32

/* Bit definitions */
typedef enum {
	bit7 = 7, bit6 = 6, bit5 = 5, bit4 = 4, bit3 = 3, bit2 = 2, bit1 = 1, bit0 = 0
} bitNum_t;

/* power enum typedef */
typedef enum {
	RF_PWR_0dBm = 0x03, RF_PWR_6dBm = 0x02, RF_PWR_12dBm = 0x01, RF_PWR_18dBm = 0x00
} powerRF_t;

/* data rate enum typedef */
typedef enum {
	RF_DataRate_250 = 0x04, RF_DataRate_1M = 0x00, RF_DataRate_2M = 0x01
} dataRate_t;

/* CRC coding */
typedef enum {
	CRC_8_bits = 0, CRC_16_bits = 1
} widthCRC_t;

/* address width typedef */
typedef enum {
	shortWidth = 0x01, mediumWidth = 0x02, longWidth = 0x03
} addressWidth_t;

/* Structures */
typedef struct {
	SPI_HandleTypeDef *nRFspi;
	TIM_HandleTypeDef *nRFtim;

	GPIO_TypeDef *nRFportCSN;
	uint16_t nRFpinCSN;
	GPIO_TypeDef *nRFportCE;
	uint16_t nRFpinCE;
} nRF24L01_hardware_struct_t;

typedef struct {
	uint8_t rxMode :1;
	uint8_t channel;
	dataRate_t dataRate;
	powerRF_t powerRF;

	uint8_t ARD; //automatic retransmissions
	uint8_t ARC; //auto retransmit count

	uint8_t pipeEn;
	uint8_t pipeACK;
	uint8_t pipeDPL;
	uint8_t pipePayLen[6];

	uint8_t enableCRC :1;
	uint8_t codingCRC :1;

	uint8_t enableTxIrq :1;
	uint8_t enableRxIrq :1;
	uint8_t enableMaxRtIrq :1;

	uint8_t enableDPL :1;
	uint8_t enableAckPay :1;
	uint8_t enableDynACK :1;
} nRF24L01_settings_struct_t;

typedef struct {
	addressWidth_t addrWidth;
	uint8_t txAddr[5]; //5 byte register

	uint8_t rxAddr0[5]; //5 byte register
	uint8_t rxAddr1[5]; //5 byte register
	uint8_t rxAddr2;    //1 byte registers
	uint8_t rxAddr3;
	uint8_t rxAddr4;
	uint8_t rxAddr5;
} nRF24L01_address_struct_t;

typedef struct {
	uint8_t txReUse :1;

	uint8_t rxRead :1;
	uint8_t rxFull :1;
	uint8_t rxEmpty :1;

	uint8_t txSend :1;
	uint8_t txFull :1;
	uint8_t txEmpty :1;
} nRF24L01_fifo_struct_t;

typedef struct {
	uint8_t *ptBufferTX;
	uint8_t *ptBufferRX;
	uint8_t tBufferTX[TX_BUFFER_SIZE];
	uint8_t tBufferRX[RX_BUFFER_SIZE];
} nRF24L01_Buffer_struct_t;

typedef struct {
	uint8_t dataReadIrq :1;
	uint8_t dataSendIrq :1;
	uint8_t maxRetr :1;
	uint8_t pipeNumber :3;
	uint8_t txFull :1;

	uint8_t packetsLost;
	uint8_t packetsRetr;
} nRF24L01_status_struct_t;

typedef struct {
	nRF24L01_hardware_struct_t hardware_struct;
	nRF24L01_settings_struct_t settings_struct;
	nRF24L01_address_struct_t address_struct;
	nRF24L01_fifo_struct_t fifo_struct;
	nRF24L01_status_struct_t status_struct;
	nRF24L01_Buffer_struct_t buffer_struct;
} nRF24L01_struct_t;

bool nRF_Init(nRF24L01_struct_t *psNRF24L01, SPI_HandleTypeDef *HAL_SPIx, TIM_HandleTypeDef *HAL_TIMx,
		GPIO_TypeDef *HAL_GPIO_CSN,
		uint16_t HAL_GPIO_Pin_CSN, GPIO_TypeDef *HAL_GPIO_CE, uint16_t HAL_GPIO_Pin_CE);


#endif /* INC_NRF24L01_H_ */
