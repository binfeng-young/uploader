//
// Created by bfyoung on 2/10/18.
//
extern "C" {
#include "BvSerial.h"
}
#include "ssp.h"
#include "fifo_buffer.h"

#include <cstdio>
#include <chrono>
#include <queue>

#define UART_BUFFER_SIZE    256
#define MAX_PACKET_DATA_LEN 255
#define MAX_PACKET_BUF_SIZE (1 + 1 + MAX_PACKET_DATA_LEN + 2)
#define BL_WAIT_TIME        6 * 1000 * 1000
#define DFU_BUFFER_SIZE     63

/// time
std::chrono::system_clock::time_point start;
uint32_t getTime();

///ssp
uint8_t ssp_dfu = FALSE;
static uint8_t rx_buffer[UART_BUFFER_SIZE];
static uint8_t txBuf[MAX_PACKET_BUF_SIZE];
static uint8_t rxBuf[MAX_PACKET_BUF_SIZE];
static void SSP_CallBack(uint8_t *buf, uint16_t len);
static int16_t SSP_SerialRead(void);
static void SSP_SerialWrite(uint8_t);
int comId = 5;

static const PortConfig_t ssp_portConfig = {
        .rxBuf         = rxBuf,
        .rxBufSize     = MAX_PACKET_DATA_LEN,
        .txBuf         = txBuf,
        .txBufSize     = MAX_PACKET_DATA_LEN,
        .max_retry     = 10,
        .timeoutLen    = 1000,
        .pfCallBack    = SSP_CallBack,
        .pfSerialRead  = SSP_SerialRead,
        .pfSerialWrite = SSP_SerialWrite,
        .pfGetTime     = getTime,
};

static Port_t ssp_port;
static t_fifo_buffer ssp_buffer;

void initTime()
{
    start = std::chrono::high_resolution_clock::now();
}

uint32_t getTime()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast< std::chrono::milliseconds>
            (std::chrono::high_resolution_clock::now() - start);
    uint32_t a = static_cast<uint32_t>(duration.count());
    printf("%d --- \n", a);
    return a;
}
int16_t status           = 0;
static uint8_t mReceive_Buffer[DFU_BUFFER_SIZE];
int main()
{
    initTime();
    if (bv_uart_open(comId, 115200) < 0) {
        return -1;
    }

    fifoBuf_init(&ssp_buffer, rx_buffer, UART_BUFFER_SIZE);
    ssp_Init(&ssp_port, &ssp_portConfig);
    int count = 0;
    while (!ssp_Synchronise(&ssp_port) && (count < 10)) {
        printf( "SYNC failed, resending...\n");
        count++;
    }
    if (count == 10) {
        printf("SYNC failed\n");
        return 0;
    }
    while (1) {
        do {
            ssp_ReceiveProcess(&ssp_port);
            status = ssp_SendProcess(&ssp_port);
        } while ((status != SSP_TX_IDLE) && (status != SSP_TX_ACKED));
        if (fifoBuf_getUsed(&ssp_buffer) >= DFU_BUFFER_SIZE) {
            for (int32_t x = 0; x < DFU_BUFFER_SIZE; ++x) {
                mReceive_Buffer[x] = fifoBuf_getByte(&ssp_buffer);
                printf("%d",mReceive_Buffer[x]);
            }
            printf("\n");
        }
        printf("time oooout");
    }
    bv_uart_close(comId);
    return 0;
}

int32_t platform_senddata(const uint8_t *msg, uint16_t msg_len)
{

    return ssp_SendData(&ssp_port, msg, msg_len);
}


//
//uint32_t callback_cnt = 0;
//uint32_t read_cnt     = 0;
//uint32_t write_cnt    = 0;
//uint32_t rx_check_cnt = 0;
void SSP_CallBack(uint8_t *buf, uint16_t len)
{
    ssp_dfu = true;
    //callback_cnt++;
    fifoBuf_putData(&ssp_buffer, buf, len);
}

int16_t SSP_SerialRead(void)
{
    char byte;

    //rx_check_cnt++;
    if (bv_uart_read(comId, &byte, 1) == 1) {
        //read_cnt++;
        return byte;
    } else {
        return -1;
    }
}

void SSP_SerialWrite(uint8_t value)
{
    //write_cnt++;
    bv_uart_write(comId, &value, 1);
}
