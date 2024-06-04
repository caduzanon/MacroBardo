#ifndef _USB_DESCRIPTORS_H_
#define _USB_DESCRIPTORS_H_

#include "tusb.h"

enum
{
  REPORT_ID_KEYBOARD = 1,
  REPORT_ID_MOUSE,
  REPORT_ID_CONSUMER_CONTROL,
  REPORT_ID_COUNT
};

// HID Report Descriptor
extern const uint8_t hid_report_descriptor[];
extern const uint16_t hid_report_descriptor_size;

const uint8_t* tud_descriptor_device_cb(void);
const uint8_t* tud_descriptor_configuration_cb(uint8_t index);
const uint16_t* tud_descriptor_string_cb(uint8_t index, uint16_t langid);
uint8_t const* tud_hid_descriptor_report_cb(uint8_t instance);
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen);
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize);

#endif // _USB_DESCRIPTORS_H_