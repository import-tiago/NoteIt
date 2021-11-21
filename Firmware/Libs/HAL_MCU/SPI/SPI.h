#ifndef SPI_H_
#define SPI_H_

#include <msp430.h>
#include <stdint.h>

void SPI_Master_Mode_Init(uint8_t eUSCI);

static void power_on(void);
static void send_initial_clock_train(void);

static void rcvr_spi_m(uint8_t *dst);
static uint8_t rcvr_spi(void);

static uint8_t wait_ready(void);
static void xmit_spi(uint8_t dat);

#endif // SPI_H_
