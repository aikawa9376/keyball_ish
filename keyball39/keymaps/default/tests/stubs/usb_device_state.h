#pragma once
typedef enum { USB_DEVICE_STATE_INIT, USB_DEVICE_STATE_CONFIGURED, USB_DEVICE_STATE_SUSPEND } usb_configure_state_t;
struct usb_device_state { usb_configure_state_t configure_state; };
void notify_usb_device_state_change_user(struct usb_device_state state);
