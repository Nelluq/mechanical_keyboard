/**
 * MIT License
 * 
 * Copyright (c) 2020 Cullen Jemison
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _USB_HID_H
#define _USB_HID_H

#include <stdint.h>

#define MAX_NUM_KEY_CODES 6

struct usb_hid_report {
    uint8_t modifiers;
    uint8_t leds;
    uint8_t key_codes[MAX_NUM_KEY_CODES];
} __attribute((packed));

void usb_hid_init(void);

void usb_hid_setup_gpio(void);

void usb_hid_send_report(struct usb_hid_report *report);

void usb_hid_poll(void);


#endif  // _USB_HID_H