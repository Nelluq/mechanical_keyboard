#include "usb_hid.h"

#include <stddef.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/st_usbfs.h>
#include <libopencm3/usb/hid.h>
#include <libopencm3/usb/usbd.h>

#define USB_HID_REPORT_DESC_SIZE 62
#define USB_HID_DT_HID_SIZE 0x09
#define USB_HID_CONFIG_TOTAL_SIZE (             \
          USB_DT_CONFIGURATION_SIZE             \
        + USB_DT_INTERFACE_SIZE                 \
        + sizeof(struct usb_hid_descriptor_full)\
        + USB_DT_ENDPOINT_SIZE )                \

#define NUM_USB_STRINGS 5

const struct usb_device_descriptor usb_device_desc = {
    .bLength = USB_DT_DEVICE_SIZE,
    .bDescriptorType = USB_DT_DEVICE,
    .bcdUSB = 0x0100,
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = 0x0483,
    .idProduct = 0x5711,
    .bcdDevice = 0x0100,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

static uint8_t usb_hid_report_desc[USB_HID_REPORT_DESC_SIZE] __attribute__((packed)) = {
    0x05, 0x01,        // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,        // USAGE (Keyboard)
    0xa1, 0x01,        // COLLECTION (Application)
    0x05, 0x07,        //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,        //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,        //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,        //   LOGICAL_MINIMUM (0)
    0x25, 0x01,        //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,        //   REPORT_SIZE (1)
    0x95, 0x08,        //   REPORT_COUNT (8)
    0x81, 0x02,        //   INPUT (Data,Var,Abs)
    0x95, 0x01,        //   REPORT_COUNT (1)
    0x75, 0x08,        //   REPORT_SIZE (8)
    0x81, 0x01,        //   INPUT (Cnst,Ary,Abs)
    0x05, 0x08,        //   USAGE_PAGE (LEDs)
    0x19, 0x01,        //   USAGE_MINIMUM (Num Lock)
    0x29, 0x03,        //   USAGE_MAXIMUM (Scroll Lock)
    0x75, 0x01,        //   REPORT_SIZE (1)
    0x95, 0x03,        //   REPORT_COUNT (3)
    0x91, 0x02,        //   OUTPUT (Data,Var,Abs)
    0x75, 0x05,        //   REPORT_SIZE (5)
    0x95, 0x01,        //   REPORT_COUNT (1)
    0x91, 0x01,        //   OUTPUT (Cnst,Ary,Abs)
    0x05, 0x07,        //   USAGE_PAGE (Keyboard)
    0x19, 0x00,        //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,        //   USAGE_MAXIMUM (Keyboard Application)
    0x26, 0xff, 0x00,  //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,        //   REPORT_SIZE (8)
    0x95, 0x06,        //   REPORT_COUNT (6)
    0x81, 0x00,        //   INPUT (Data,Ary,Abs)
    0xc0               // END_COLLECTION
};

static struct usb_hid_descriptor_full {
    struct usb_hid_descriptor head;
    uint8_t bDescriptorType;
    uint16_t wDescriptorLength;
} __attribute__((packed));

const struct usb_hid_descriptor_full usb_hid_desc = {
    .head = {
        .bLength = USB_HID_DT_HID_SIZE,
        .bDescriptorType = USB_HID_DT_HID,
        .bcdHID = 0x0111,
        .bCountryCode = 0,
        .bNumDescriptors = 1,
    },
    .bDescriptorType = USB_HID_DT_REPORT,
    .wDescriptorLength = USB_HID_REPORT_DESC_SIZE,
};

const struct usb_endpoint_descriptor usb_endpoint_desc = {
    .bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_ENDPOINT_ADDR_IN(1),
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 0x0008,
	.bInterval = 5,  // 5ms - 200Hz
};

const struct usb_interface_descriptor usb_iface_desc = {
    .bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_HID,
	.bInterfaceSubClass = USB_HID_SUBCLASS_NO,
	.bInterfaceProtocol = USB_HID_INTERFACE_PROTOCOL_KEYBOARD,
	.iInterface = 5,

    // reference the above endpoint descriptor
    .endpoint = &usb_endpoint_desc,
    
    // reference the above HID descriptor
    .extra = &usb_hid_desc,
    .extralen = sizeof(usb_hid_desc),
};

const struct usb_interface usb_iface = {
    .num_altsetting = 1,
    .altsetting = &usb_iface_desc,
};

const struct usb_config_descriptor usb_config_desc = {
    .bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = USB_HID_CONFIG_TOTAL_SIZE,
	.bNumInterfaces = 1,
	.bConfigurationValue = 1,
	.iConfiguration = 4,
	.bmAttributes = USB_CONFIG_ATTR_DEFAULT | USB_CONFIG_ATTR_REMOTE_WAKEUP,
	.bMaxPower = 50,  // 100mA

    // reference the above interface
    .interface = &usb_iface
};

const char *usb_strings[NUM_USB_STRINGS] = {
    "Cullen Jemison",
    "keyBOARD",
    "rev3",
    "Keyboard Configuration",
    "Keyboard Interface",
};

static usbd_device *usb_dev;

static uint8_t usb_control_buf[128];

static enum usbd_request_return_codes usb_hid_control_cb(usbd_device *usbd_dev, struct usb_setup_data *req,
    uint8_t **buf, uint16_t *len, usbd_control_complete_callback *complete) {
    
    // respond to the HID report descriptor request
    if (
        (req->bmRequestType & (USB_REQ_TYPE_DIRECTION | USB_HID_REQ_TYPE_GET_REPORT)) &&
        (req->bRequest == USB_REQ_GET_DESCRIPTOR) &&
        (req->wValue = USB_HID_DT_REPORT << 8)
    ) {
        *buf = usb_hid_report_desc;
        *len = USB_HID_REPORT_DESC_SIZE;

        return USBD_REQ_HANDLED;
    }

    (void)usbd_dev;
    (void)complete;

    return USBD_REQ_NOTSUPP;
}

static void usb_set_config(usbd_device *dev, uint16_t wValue) {
    // setup the keyboard configuration regardless of wValue (since it's the only one)
    usbd_ep_setup(dev, usb_endpoint_desc.bEndpointAddress, usb_endpoint_desc.bmAttributes,
        usb_endpoint_desc.wMaxPacketSize, NULL);

    // setup the HID control callback
    usbd_register_control_callback(dev, (USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE),
        (USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT), usb_hid_control_cb);

    (void)wValue;
}

void usb_hid_init(void) {
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_USB);

    rcc_set_usbclk_source(RCC_PLL);

    // setup the USB device
    usb_dev = usbd_init(&st_usbfs_v2_usb_driver, &usb_device_desc, &usb_config_desc, usb_strings,
        NUM_USB_STRINGS, usb_control_buf, sizeof(usb_control_buf));
    usbd_register_set_config_callback(usb_dev, usb_set_config);
}

void usb_hid_setup_gpio(void) {
    
}

void usb_hid_send_report(struct usb_hid_report *report) {
    usbd_ep_write_packet(usb_dev, usb_endpoint_desc.bEndpointAddress, report, sizeof(struct usb_hid_report));
}

void usb_hid_poll(void) {
    usbd_poll(usb_dev);
}
