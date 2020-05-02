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