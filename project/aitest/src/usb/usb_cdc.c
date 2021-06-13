/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "esp_task.h"

#include "tusb.h"
#include "usb_cdc.h"
#include "tinyusb.h"
#include "esp_log.h"

#define DBG_MAX_PACKET      (64)
#define IDE_BAUDRATE_SLOW   (921600)
#define IDE_BAUDRATE_FAST   (12000000)

// MicroPython runs as a task under FreeRTOS
#define USB_CDC_TASK_PRIORITY        (ESP_TASK_PRIO_MIN + 1)
#define USB_CDC_TASK_STACK_SIZE      (16 * 1024)

#define USB_TASK_PRIORITY            (ESP_TASK_PRIO_MIN + 1)
#define USB_TASK_STACK_SIZE          (16 * 1024)

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

volatile uint8_t  dbg_mode_enabled = 1;

extern void usbdbg_data_in(void *buffer, int length);
extern void usbdbg_data_out(void *buffer, int length);
extern void usbdbg_control(void *buffer, uint8_t brequest, uint32_t wlength);

uint8_t rx_ringbuf_array[1024];
uint8_t tx_ringbuf_array[1024];
volatile ringbuf_t rx_ringbuf;
volatile ringbuf_t tx_ringbuf;

static void cdc_task(void);

static bool cdc_rx_any(void) {
    return rx_ringbuf.iput != rx_ringbuf.iget;
}

static int cdc_rx_char(void) {
    return ringbuf_get((ringbuf_t*)&rx_ringbuf);
}

static bool cdc_tx_any(void) {
    return tx_ringbuf.iput != tx_ringbuf.iget;
}

static int cdc_tx_char(void) {
    return ringbuf_get((ringbuf_t*)&tx_ringbuf);
}

uint32_t usb_cdc_buf_len()
{
    return ringbuf_avail((ringbuf_t*)&tx_ringbuf);
}

uint32_t usb_cdc_get_buf(uint8_t *buf, uint32_t len)
{
    int i=0;
    for (; i<len; i++) {
        buf[i] = ringbuf_get((ringbuf_t*)&tx_ringbuf);
        if (buf[i] == -1) {
            break;
        }
    }
    return i;
}

static void cdc_task(void)
{
    if ( tud_cdc_connected() ) {
        // connected and there are data available
        while (tud_cdc_available()) {
            int c;
            uint32_t count = tud_cdc_read(&c, 1);
            (void)count;
            ringbuf_put((ringbuf_t*)&rx_ringbuf, c);
        }

        int chars = 0;
        while (cdc_tx_any()) {
            if (chars < 64) {
               tud_cdc_write_char(cdc_tx_char());
               chars++;
            } else {
               chars = 0;
               tud_cdc_write_flush();
            }
        }
        tud_cdc_write_flush();
    }
}

static void cdc_task_debug_mode(void)
{
    if ( tud_cdc_connected() && tud_cdc_available() ) {
        uint8_t buf[DBG_MAX_PACKET];
        uint32_t count = tud_cdc_read(buf, 6);
        if (count < 6) {
            //Shouldn't happen
            return;
        }
        // assert buf[0] == '\x30';
        uint8_t request = buf[1];
        uint32_t xfer_length = *((uint32_t*)(buf+2));
        usbdbg_control(buf+6, request, xfer_length);

        while (xfer_length) {
            if (request & 0x80) {
                // Device-to-host data phase
                int bytes = MIN(xfer_length, DBG_MAX_PACKET);
                if (bytes <= tud_cdc_write_available()) {
                    xfer_length -= bytes;
                    usbdbg_data_in(buf, bytes);
                    tud_cdc_write(buf, bytes);
                }
                tud_cdc_write_flush();
            } else {
                // Host-to-device data phase
                int bytes = MIN(xfer_length, DBG_MAX_PACKET);
                uint32_t count = tud_cdc_read(buf, bytes);
                if (count == bytes) {
                    xfer_length -= count;
                    usbdbg_data_out(buf, count);
                }
            }
        }
    }
}

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding)
{
    if (p_line_coding->bit_rate == IDE_BAUDRATE_SLOW ||
            p_line_coding->bit_rate == IDE_BAUDRATE_FAST) {
        dbg_mode_enabled = 1;
    } else {
        dbg_mode_enabled = 0;
    }
    tx_ringbuf.iget = 0;
    tx_ringbuf.iput = 0;

    rx_ringbuf.iget = 0;
    rx_ringbuf.iput = 0;
}

void usb_cdc_loop(void) {
    while(true){
        if (dbg_mode_enabled == true) {
            cdc_task_debug_mode();
        }else {
            cdc_task();
        }
    }
}

void tusb_device_task(void)
{
    while (1) {            
        if (tud_task_event_ready()) {
            tud_task();
        }
    }
}

int usb_cdc_init(void)
{
    static bool initialized = false;
    if (!initialized) {
        initialized = true;

        rx_ringbuf.buf = rx_ringbuf_array;
        rx_ringbuf.size = sizeof(rx_ringbuf_array);
        rx_ringbuf.iget = 0;
        rx_ringbuf.iput = 0;

        tx_ringbuf.buf = tx_ringbuf_array;
        tx_ringbuf.size = sizeof(tx_ringbuf_array);
        tx_ringbuf.iget = 0;
        tx_ringbuf.iput = 0;
        //tusb_init();
        tinyusb_config_t tusb_cfg = {
            .descriptor = NULL,
            .string_descriptor = NULL,
            .external_phy = false // In the most cases you need to use a `false` value
        };

        ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
        // Create a task for tinyusb device stack
        xTaskCreatePinnedToCore(usb_cdc_loop, "usb_cdc", USB_CDC_TASK_STACK_SIZE / sizeof(StackType_t), NULL, USB_CDC_TASK_PRIORITY, NULL, 0);
    }
    return 0;
}
