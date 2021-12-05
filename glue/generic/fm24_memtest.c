
#ifdef __I2C_CONFIG_H__

#include <string.h>
#include "fm24_memtest.h"
#include "fm24.h"
#include "glue.h"

uint32_t FM24_memtest(const uint16_t usci_base_addr, const uint8_t slave_addr, const uint32_t start_addr,
                      const uint32_t stop_addr, FM24_test_t test, uint32_t * rows_tested)
{
    uint8_t data_r[8];          // test 8 bytes (1 row) at a time
    uint8_t data_w[8];
    char itoa_buf[CONV_BASE_10_BUF_SZ];
    uint32_t fragments, remain;
    uint32_t rv = 0;
    uint32_t i;
    uint8_t j;

    *rows_tested = 0;

    switch (test) {
    case TEST_00:
        memset(data_w, 0x00, 8);
        break;
    case TEST_FF:
        memset(data_w, 0xff, 8);
        break;
    case TEST_AA:
        memset(data_w, 0xaa, 8);
        break;
    case TEST_CNT:
        for (i=0; i<8; i++) {
            data_w[i] = i+0xa1;
        }
        break;
    }

    fragments = (stop_addr - start_addr) / 8;
    remain = (stop_addr - start_addr) % 8;

    for (i = 0; i < fragments; i++) {
        FM24_write(usci_base_addr, slave_addr, &data_w[0], (i * 8) + start_addr, 8);
        memset(data_r, 0x11, 8);
        FM24_read(usci_base_addr, slave_addr, &data_r[0], (i * 8) + start_addr, 8);
        for (j = 0; j < 8; j++) {
            if (data_w[j] != data_r[j]) {
                uart0_print(_utoh32(itoa_buf, data_w[j]));
                uart0_print(" != ");
                uart0_print(_utoh32(itoa_buf, data_r[j]));
                uart0_print("\r\n");
                rv++;
            }
        }
        *rows_tested += 1;
    }

    if (remain) {
        FM24_write(usci_base_addr, slave_addr, data_w, (fragments * 8) + start_addr, remain);
        memset(data_r, 0x11, 8);
        FM24_read(usci_base_addr, slave_addr, data_r, (fragments * 8) + start_addr, remain);
        for (j = 0; j < remain; j++) {
            if (data_w[j] != data_r[j]) {
                uart0_print("~");
                rv++;
            }
        }
        *rows_tested += 1;
    }

    return rv;
}

#endif // __I2C_CONFIG_H__

