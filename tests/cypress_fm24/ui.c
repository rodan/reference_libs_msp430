
#include <stdio.h>
#include <string.h>
#include "glue.h"
#include "version.h"
#include "uart_mapping.h"
#include "ui.h"

void display_memtest(const uint16_t usci_base_addr, const uint8_t slave_addr, const uint32_t start_addr, const uint32_t stop_addr, FM24_test_t test)
{
    uint32_t el;
    uint32_t rows_tested;
    char itoa_buf[CONV_BASE_10_BUF_SZ];

    uart_bcl_print(" \e[36;1m*\e[0m testing ");
    uart_bcl_print(_utoh32(itoa_buf, start_addr));
    uart_bcl_print(" - ");
    uart_bcl_print(_utoh32(itoa_buf, stop_addr));
    uart_bcl_print(" with pattern #");
    uart_bcl_print(_utoa(itoa_buf, test));
    uart_bcl_print("    ");

    el = FM24_memtest(usci_base_addr, slave_addr, start_addr, stop_addr, test, &rows_tested);

    uart_bcl_print(_utoa(itoa_buf, rows_tested * 8));
    if (el == 0) { 
        uart_bcl_print(" bytes tested \e[32;1mok\e[0m\r\n");
    } else {
        uart_bcl_print(" bytes tested with \e[31;1m");
        uart_bcl_print(_utoa(itoa_buf, el));
        uart_bcl_print(" errors\e[0m\r\n");
    }
}

void print_buf_fram(const uint32_t address, const uint32_t size)
{
    uint32_t bytes_remaining = size;
    uint32_t bytes_to_be_printed, bytes_printed = 0;
    char itoa_buf[CONV_BASE_10_BUF_SZ];
    uint16_t i;
    uint8_t *read_ptr = NULL;
    uint8_t row[16];

    while (bytes_remaining > 0) {
    
        if (bytes_remaining > 16) {
            bytes_to_be_printed = 16;
        } else {
            bytes_to_be_printed = bytes_remaining;
        }

        uart_bcl_print(_utoh32(itoa_buf, bytes_printed));
        uart_bcl_print(": ");

        FM24_read(I2C_BASE_ADDR, FM24_SLAVE_ADDR, row, address + bytes_printed, bytes_to_be_printed);
        read_ptr = &row[0];

        for (i = 0; i < bytes_to_be_printed; i++) {
            uart_bcl_print(_utoh8(itoa_buf, *read_ptr++));
            if (i & 0x1) {
                uart_bcl_print(" ");
            }
        }
        uart_bcl_print(" ");

        read_ptr = &row[0];
        for (i = 0; i < bytes_to_be_printed; i++) {
            uart_bcl_tx_str((char *)read_ptr++, 1);
        }

        uart_bcl_print("\r\n");
        bytes_printed += bytes_to_be_printed;
        bytes_remaining -= bytes_to_be_printed;
    }
}


static const char menu_str[]="\
 available commands:\r\n\r\n\
 \e[33;1m?\e[0m - show menu\r\n\
 \e[33;1mt\e[0m - memtest\r\n\
 \e[33;1mr\e[0m - tiny read test\r\n\
 \e[33;1mw\e[0m - tiny write test\r\n\
 \e[33;1mh\e[0m - hex dump of FRAM segment\r\n";

void display_menu(void)
{
    display_version();
    uart_bcl_print(menu_str);
}

void display_version(void)
{
    char sconv[CONV_BASE_10_BUF_SZ];

    uart_bcl_print("fm24 b");
    uart_bcl_print(_utoa(sconv, BUILD));
    uart_bcl_print("\r\n");
}

#define PARSER_CNT 16

void parse_user_input(void)
{
#if defined UART0_RX_USES_RINGBUF || defined UART1_RX_USES_RINGBUF || \
    defined UART2_RX_USES_RINGBUF || defined UART3_RX_USES_RINGBUF
    struct ringbuf *rbr = uart_bcl_get_rx_ringbuf();
    uint8_t rx;
    uint8_t c = 0;
    char input[PARSER_CNT];

    memset(input, 0, PARSER_CNT);

    // read the entire ringbuffer
    while (ringbuf_get(rbr, &rx)) {
        if (c < PARSER_CNT-1) {
            input[c] = rx;
        }
        c++;
    }
#else
    char *input = uart_bcl_get_rx_buf();
#endif
    char f = input[0];
    //char itoa_buf[CONV_BASE_10_BUF_SZ];
    uint8_t foo[9]="hello wo";
    uint8_t bar[9]="33333333";
    uint8_t data_r[8]; // test 8 bytes (1 row) at a time

    if (f == '?') {
        display_menu();
    } else if (f == 'w') {
        FM24_write(I2C_BASE_ADDR, FM24_SLAVE_ADDR, foo, FM_LA-20, 8);
    } else if (f == 'b') {
        FM24_write(I2C_BASE_ADDR, FM24_SLAVE_ADDR, bar, 0x90, 8);
    } else if (f == 'r') {
        FM24_read(I2C_BASE_ADDR, FM24_SLAVE_ADDR, data_r, FM_LA-20, 8);
    } else if (f == 't') {
        //display_memtest(I2C_BASE_ADDR, 0xffe0, FM_LA, TEST_00);
        //display_memtest(I2C_BASE_ADDR, 0xffe0, FM_LA, TEST_00);
        //display_memtest(I2C_BASE_ADDR, 0x10, 0xb0, TEST_AA);
        //display_memtest(I2C_BASE_ADDR, 0x40, 0x60, TEST_FF);
        //display_memtest(I2C_BASE_ADDR, 0x60, 0x80, TEST_AA);
        //display_memtest(I2C_BASE_ADDR, 0x90, 0x98, TEST_00);

        display_memtest(I2C_BASE_ADDR, FM24_SLAVE_ADDR, 0, FM_LA, TEST_FF);
        display_memtest(I2C_BASE_ADDR, FM24_SLAVE_ADDR, 0, FM_LA, TEST_CNT);
        display_memtest(I2C_BASE_ADDR, FM24_SLAVE_ADDR, 0, FM_LA, TEST_00);
        uart_bcl_print(" * roll over test\r\n");
        display_memtest(I2C_BASE_ADDR, FM24_SLAVE_ADDR, FM_LA - 3, FM_LA + 5, TEST_CNT);
    } else if (f == '1') {
        FM24_read(I2C_BASE_ADDR, FM24_SLAVE_ADDR, data_r, FM_LA-3, 8);
    } else if (f == '2') {
        display_memtest(I2C_BASE_ADDR, FM24_SLAVE_ADDR, FM_LA - 3, FM_LA + 5, TEST_CNT);
    } else if (f == 'h') {
        print_buf_fram(FM_LA - 63, 128);
    } 
}
