/**
 * Created by bfyoung on 2018/2/9.
 */

extern "C" {
#include "BvSerial.h"
}

#include <cstdio>
int main()
{
    int comId = 3;
    if (bv_uart_open(comId, 115200) < 0) {
        return -1;
    }
    char buf[40] = "123\0";
    bv_uart_write(comId, buf, 3);
    char buf2[40] = {0};
    if (bv_uart_read(comId, buf2, 3) > 0) {
        printf("-----%s\n", buf2);
    } else {
        printf("read error");
    }
    bv_uart_close(comId);
    return 0;
}