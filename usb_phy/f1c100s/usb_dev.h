/*
 * usb_dev.h
 *
 *  Created on: 2018��10��5��
 *      Author: VeiLiang
 */

#ifndef DRIVER_INCLUDE_USB_DEV_H_
#define DRIVER_INCLUDE_USB_DEV_H_

enum ep0_state {
	EP0_IDLE,
	EP0_IN_DATA_PHASE,
	EP0_OUT_DATA_PHASE,
	EP0_END_XFER,
	EP0_STALL,
};
enum usb_device_speed {
	USB_SPEED_UNKNOWN = 0,			/* enumerating */
	USB_SPEED_LOW, USB_SPEED_FULL,		/* usb 1.1 */
	USB_SPEED_HIGH				/* usb 2.0 */
};
enum usb_xfer_type_req{
	NONE_REQ,
	SET_ADDR,
	SET_CONFIG,
};
/*  usb transfer type  */
#define  USBC_TS_TYPE_IDLE                  0
#define  USBC_TS_TYPE_CTRL                  1
#define  USBC_TS_TYPE_ISO                   2
#define  USBC_TS_TYPE_INT                   3
#define  USBC_TS_TYPE_BULK                  4
extern unsigned int usb_connect;
int usb_ep0_state;
int usb_device_write_data_ep_pack(int ep,unsigned char * databuf,int len);
int usb_device_read_data_ep_pack(int ep,unsigned char * databuf,int len);
void usb_device_send_nullpack_ep0(void);
void usb_device_read_data_status_ep0(unsigned char is_complete);
void usb_device_clear_setup_end(void);
void usb_device_set_address(unsigned char addr);
void usb_device_set_ep0_state(enum ep0_state state);
void usb_device_set_xfer_type(enum usb_xfer_type_req type);
void usb_config_ep_out(int epidx,int maxpack,int type);
void usb_config_ep_in(int epidx,int maxpack,int type);
void usb_device_clear_ep_halt(int epaddr);
void usb_reg_debugdump(void);
#endif /* DRIVER_INCLUDE_USB_DEV_H_ */
