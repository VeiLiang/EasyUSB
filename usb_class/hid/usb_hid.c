#include "usb.h"
#include "usb_dev.h"
/*
 * USB types, the second of three bRequestType fields
 */
#define USB_TYPE_MASK                   (0x03 << 5)
#define USB_TYPE_STANDARD               (0x00 << 5)
#define USB_TYPE_CLASS                  (0x01 << 5)
#define USB_TYPE_VENDOR                 (0x02 << 5)
#define USB_TYPE_RESERVED               (0x03 << 5)

/*
 * HID class requests
 */
#define HID_REQ_GET_REPORT          0x01
#define HID_REQ_GET_IDLE            0x02
#define HID_REQ_GET_PROTOCOL        0x03
#define HID_REQ_SET_REPORT          0x09
#define HID_REQ_SET_IDLE            0x0A
#define HID_REQ_SET_PROTOCOL        0x0B


static USB_DeviceRequest usb_setup_request;

static USB_DeviceDescriptor hid_kybDesc =
{
	sizeof(USB_DeviceDescriptor),
	DEVICE_DESCRIPTOR,	//1
//    #if DWC_FORCE_SPEED_FULL
	0x0110,     //Version 1.11
//	#else
//	0x0200,     //Version 2.0
//    #endif
	0x00,
	0x00,
	0x00,
	64,	/* Ep0 FIFO size */
	0x0483,
	0x5741,
	USB_FUNCTION_RELEASE_NO,
	0x01, 	//iManufacturer;
	0x02,   //iProduct;
	0x00,
	0x01
};
static USB_DeviceDescriptor hid_posDesc =
{
	sizeof(USB_DeviceDescriptor),
	DEVICE_DESCRIPTOR,	//1
	0x0110,     //Version 1.11
	0x00,
	0x00,
	0x00,
	64,	/* Ep0 FIFO size */
	0x2f50,
	0x0300,
	0x0100,
	0x01, 	//iManufacturer;
	0x02,   //iProduct;
	0x00,
	0x01
};
//下面这这几项描述了一个输入用的字段，总共为8个bits，每个bit表示一个按键
    //分别从左ctrl键到右GUI键。这8个bits刚好构成一个字节，它位于报告的第一个字节。
    //它的最低位，即bit-0对应着左ctrl键，如果返回的数据该位为1，则表示左ctrl键被按下，
    //否则，左ctrl键没有按下。最高位，即bit-7表示右GUI键的按下情况。中间的几个位，
    //需要根据HID协议中规定的用途页表（HID Usage Tables）来确定。这里通常用来表示
    //特殊键，例如ctrl，shift，del键等
static u8 hid_keyboard_report[] = {
	0x05, 0x01,
	0x09, 0x06,
	0xa1, 0x01,
	0x05, 0x07,
	0x19, 0xe0,
	0x29, 0xe7,
	0x15, 0x00,
	0x25, 0x01,
	0x95, 0x08,
	0x75, 0x01,
	0x81, 0x02,
	0x95, 0x01,
	0x75, 0x08,
	0x81, 0x03,
	0x95, 0x06,
	0x75, 0x08,
	0x15, 0x00,
	0x25, 0xff,
	0x05, 0x07,
	0x19, 0x00,
	0x29, 0x65,
	0x81, 0x00,
	0x25, 0x01,
	0x95, 0x05,
	0x75, 0x01,
	0x05, 0x08,
	0x19, 0x01,
	0x29, 0x05,
	0x91, 0x02,
	0x95, 0x01,
	0x75, 0x03,
	0x91, 0x03,
	0xc0
};
static u8 hid_pos_report[] = {
		0x05, 0x8c,
		0x09, 0x01,
		0xa1, 0x01,
		0x09, 0x07,
		0xa1, 0x02,
		0x85, 0x01,
		0x15, 0x00,
		0x26, 0xff, 0x00,
		0x75, 0x08,
		0x95, 0x3f,
		0x09, 0xfe,
		0x82, 0x02, 0x01,
		0xc0,

		0x09, 0x08,
		0xa1, 0x02,
		0x85, 0x02,
		0x15, 0x00,
		0x26, 0xff, 0x00,
		0x75, 0x08,
		0x95, 0x3f,
		0x09, 0xfe,
		0x82, 0x02,
		0x01, 0xc0,
		0x09, 0x06,
		0xa1, 0x02,
		0x85, 0x04,
		0x15, 0x00,
		0x26, 0xff,
		0x00, 0x75,
		0x08, 0x95,
		0x3f, 0x09,
		0x00, 0x91,0x86,
		0xc0,
		0xc0
};

#define	CONFIG_HID_DESCRIPTOR_LEN	(sizeof(USB_ConfigDescriptor) + \
				 sizeof(USB_InterfaceDescriptor) + \
				 sizeof(USB_HID_Descriptor) + \
				 sizeof(USB_EndPointDescriptor) * 2)

static struct {
	USB_ConfigDescriptor    configuration_descriptor;
	USB_InterfaceDescriptor hid_control_interface_descritor;
	USB_HID_Descriptor hid_header_descriptor;
	USB_EndPointDescriptor  hid_endpoint_descriptor[2];
} __attribute__ ((packed)) hid_kyb_confDesc = {
	{
		sizeof(USB_ConfigDescriptor),
		CONFIGURATION_DESCRIPTOR,
		CONFIG_HID_DESCRIPTOR_LEN,  /*  Total length of the Configuration descriptor */
		0x01,                   /*  NumInterfaces */
		0x01,                   /*  Configuration Value */
		0x00,                   /* Configuration Description String Index */
		0x80,	// Self Powered, no remote wakeup
		0x32	// Maximum power consumption 500 mA
	},
	{
		sizeof(USB_InterfaceDescriptor),
		INTERFACE_DESCRIPTOR,
		0x00,   /* bInterfaceNumber */
		0x00,   /* bAlternateSetting */
		0x02,	/* ep number */
		0x03,   /* Interface Class */
		0x00,      /* Interface Subclass*/
		0x00,      /* Interface Protocol*/
		0x02    /* Interface Description String Index*/
	},
	{
		sizeof(USB_HID_Descriptor),
		0x21,
		0x0110,
		0x21,
		0x01,
		0x22,
		sizeof(hid_keyboard_report),
	},
	{
		{
			sizeof(USB_EndPointDescriptor),
			ENDPOINT_DESCRIPTOR,
			(1 << 7) | 2,// endpoint 3 IN
			3, /* interrupt */
			8, /* IN EP FIFO size */
			5
		},
		{
			sizeof(USB_EndPointDescriptor),
			ENDPOINT_DESCRIPTOR,
			(0 << 7) | 1,// endpoint 3 OUT
			3, /* interrupt */
			8, /* OUT EP FIFO size */
			10

		}
	},
},
hid_pos_confDesc = {
	{
		sizeof(USB_ConfigDescriptor),
		CONFIGURATION_DESCRIPTOR,
		CONFIG_HID_DESCRIPTOR_LEN,  /*  Total length of the Configuration descriptor */
		0x01,                   /*  NumInterfaces */
		0x01,                   /*  Configuration Value */
		0x00,                   /* Configuration Description String Index */
		0x80,	// Self Powered, no remote wakeup
		0x32	// Maximum power consumption 500 mA
	},
	{
		sizeof(USB_InterfaceDescriptor),
		INTERFACE_DESCRIPTOR,
		0x00,   /* bInterfaceNumber */
		0x00,   /* bAlternateSetting */
		0x02,	/* ep number */
		0x03,   /* Interface Class */
		0x00,      /* Interface Subclass*/
		0x00,      /* Interface Protocol*/
		0x02    /* Interface Description String Index*/
	},
	{
		sizeof(USB_HID_Descriptor),
		0x21,
		0x0110,
		0x21,
		0x01,
		0x22,
		sizeof(hid_pos_report),
	},
	{
		{
			sizeof(USB_EndPointDescriptor),
			ENDPOINT_DESCRIPTOR,
			(1 << 7) | 2,// endpoint 3 IN
			3, /* interrupt */
			64, /* IN EP FIFO size */
			10
		},
		{
			sizeof(USB_EndPointDescriptor),
			ENDPOINT_DESCRIPTOR,
			(0 << 7) | 1,// endpoint 3 OUT
			3, /* interrupt */
			64, /* OUT EP FIFO size */
			10

		}
	},
};

static unsigned short str_ret[] = {
			0x0326,
			'H','I','D','-','K','Y ','B ',' ','U','s','b',' ','D','e','v','i','c','e'
		};

		static unsigned short str_lang[] = {
			0x0304,
			0x0409
		};

		static unsigned short str_isernum[] = {
				0x033e,
				'U','S','B',' ','K','e','y','b','o','a','r','d',' ','d','r','i ','v','e','r','s',' ','B','y','-','-','L','i','a','n','g'
		};

typedef struct {
	unsigned char value;
	unsigned char flag;
}HidKeyAsciiStu;
const HidKeyAsciiStu EN_keyboard_value[256] = {
				['\n'] = {0x28,0},['\r'] = {0x28,0},[' '] = {0x2c,0},[0x09] = {0x2b,0},[0x1b] = {0x29,0},[0x7f] = {0x63,0x00},

				['~'] = {0x35,2},['!'] = {0x1e,2},['@'] = {0x1f,2},['#'] = {0x20,2},['$'] = {0x21,2},['%'] = {0x22,2},['^'] = {0x23,2},
				['&'] = {0x24,2},['*'] = {0x25,2},['('] = {0x26,2},[')'] = {0x27,2},['-'] = {0x2d,0},['='] = {0x2e,0},/*1 up*/

				['`'] = {0x35,0},['1'] = {0x1e,0},['2'] = {0x1f,0},['3'] = {0x20,0},['4'] = {0x21,0},['5'] = {0x22,0},['6'] = {0x23,0},
				['7'] = {0x24,0},['8'] = {0x25,0},['9'] = {0x26,0},['0'] = {0x27,0},['_'] = {0x2d,2},['+'] = {0x2e,2},/*1 down*/

				['q'] = {0x14,0},['w'] = {0x1a,0},['e'] = {0x08,0},['r'] = {0x15,0},['t'] = {0x17,0},['y'] = {0x1c,0},['u'] = {0x18,0},
				['i'] = {0x0c,0},['o'] = {0x12,0},['p'] = {0x13,0},['['] = {0x2f,0},[']'] = {0x30,0},['\\'] = {0x31,0},/*2 up*/

				['Q'] = {0x14,2},['W'] = {0x1a,2},['E'] = {0x08,2},['R'] = {0x15,2},['T'] = {0x17,2},['Y'] = {0x1c,2},['U'] = {0x18,2},
				['I'] = {0x0c,2},['O'] = {0x12,2},['P'] = {0x13,2},['{'] = {0x2f,2},['}'] = {0x30,2},['|'] = {0x31,2},/*2 down*/

				['a'] = {0x04,0},['s'] = {0x16,0},['d'] = {0x07,0},['f'] = {0x09,0},['g'] = {0x0a,0},['h'] = {0x0b,0},['j'] = {0x0d,0},
				['k'] = {0x0e,0},['l'] = {0x0f,0},[';'] = {0x33,0},['\''] = {0x34,0},/*3 up*/

				['A'] = {0x04,2},['S'] = {0x16,2},['D'] = {0x07,2},['F'] = {0x09,2},['G'] = {0x0a,2},['H'] = {0x0b,2},['J'] = {0x0d,2},
				['K'] = {0x0e,2},['L'] = {0x0f,2},[':'] = {0x33,2},['"'] = {0x34,2},/*3 down*/

				['z'] = {0x1d,0},['x'] = {0x1b,0},['c'] = {0x06,0},['v'] = {0x19,0},['b'] = {0x05,0},['n'] = {0x11,0},['m'] = {0x10,0},
				[','] = {0x36,0},['.'] = {0x37,0},['/'] = {0x38,0},/*4 up*/

				['Z'] = {0x1d,2},['X'] = {0x1b,2},['C'] = {0x06,2},['V'] = {0x19,2},['B'] = {0x05,2},['N'] = {0x11,2},['M'] = {0x10,2},
				['<'] = {0x36,2},['>'] = {0x37,2},['?'] = {0x38,2},/*4 down*/

};
static void usb_get_device_descriptor(USB_DeviceRequest *req_data)
{
    unsigned int len = req_data->wLength;
    usbprint("usb_get_device_descriptor\n");
    //if(is_hid_kyb)
    {
		if (len < sizeof(USB_DeviceDescriptor))
		{
			usb_device_write_data_ep_pack(0, (unsigned char *)&hid_kybDesc, len);
		}
		else
		{
			usb_device_write_data_ep_pack(0, (unsigned char *)&hid_kybDesc, sizeof(USB_DeviceDescriptor));
		}
	}
//    else
//    {
//    	if (size < sizeof(USB_DeviceDescriptor))
//			usb_device_write_data_ep_pack(0, (unsigned char *)&hid_posDesc, size);
//		else
//			usb_device_write_data_ep_pack(0, (unsigned char *)&hid_posDesc, sizeof(USB_DeviceDescriptor));
//    }
}

static void usb_get_cfg_descriptor(USB_DeviceRequest *req_data)
{
	usbprint("usb_get_cfg_descriptor\n");
	//if(is_hid_kyb)
	{
		switch(req_data->wLength)
		{
			case 9:
				usb_device_write_data_ep_pack(0,  (unsigned char *)&hid_kyb_confDesc, 9);
				break;
			case 8:
				usb_device_write_data_ep_pack(0,  (unsigned char *)&hid_kyb_confDesc, 8);
				break;
			default:
				usb_device_write_data_ep_pack(0,  (unsigned char *)&hid_kyb_confDesc, sizeof(hid_kyb_confDesc));
				break;
		}
	}
//	else
//	{
//		switch(req_data->wLength) {
//		case 9:
//			usb_device_write_data_ep_pack(0,  (unsigned char *)&hid_pos_confDesc, 9);
//			break;
//		case 8:
//			usb_device_write_data_ep_pack(0,  (unsigned char *)&hid_pos_confDesc, 8);
//			break;
//		default:
//			usb_device_write_data_ep_pack(0,  (unsigned char *)&hid_pos_confDesc, sizeof(hid_pos_confDesc));
//			break;
//		}
//	}
}

static inline void udp_get_dev_descriptor_string(USB_DeviceRequest *dreq)
{
	int len = dreq->wLength;
	int index = dreq->wValue & 0xff;
	usbprint("udp_get_dev_descriptor_string:%d\n",index);
	switch ( index)
	{
		case 0:
			if ( len > sizeof(str_lang) )
			{
				usb_device_write_data_ep_pack(0, (unsigned char *)str_lang,sizeof(str_lang));
			}
			else
			{
				usb_device_write_data_ep_pack(0, (unsigned char *)str_lang,len);
			}
			return;
			break;
		case 1:
			if (len >= sizeof(str_isernum))
			{
				len = sizeof(str_isernum);
			}
			usb_device_write_data_ep_pack(0, (unsigned char *)str_isernum,len);
			break;
		case 2:
			if (len >= 38)
			{
				len = 38;
			}
			str_ret[0] = (0x0300 | len);
			usb_device_write_data_ep_pack(0, (unsigned char *)str_ret,len);
			break;
		default:
			if (len >= sizeof(str_isernum))
			{
				len = sizeof(str_isernum);
			}
			str_isernum[0] = (0x0300 | len);
			usb_device_write_data_ep_pack(0, (unsigned char *)str_isernum,len);
			break;
	}
}

static void usb_get_hid_descriptor(USB_DeviceRequest *req_data)
{
	usbprint("usb_get_hid_descriptor\n");
	//if(is_hid_kyb)
	{
		usb_device_write_data_ep_pack(0, (unsigned char *)&hid_kyb_confDesc.hid_header_descriptor, sizeof(USB_HID_Descriptor));
	}
//	else
//	{
//		usb_device_write_data_ep_pack(0, (unsigned char *)&hid_pos_confDesc.hid_header_descriptor, sizeof(USB_HID_Descriptor));
//	}
}

static void usb_get_report_descriptor(USB_DeviceRequest *req_data)
{
	usbprint("usb_get_report_descriptor\n");
	//if(is_hid_kyb)
	{
		usb_device_write_data_ep_pack(0,  (unsigned char *)&hid_keyboard_report, sizeof(hid_keyboard_report));

	}
//	else
//	{
//		usb_device_write_data_ep_pack(0,  (unsigned char *)&hid_pos_report, sizeof(hid_pos_report));
//	}
}
static void usb_descriptor_request(USB_DeviceRequest* req_data)
{
	unsigned char  wValue = req_data->wValue >> 8;
	switch(wValue)
	{
		case DEVICE_DESCRIPTOR ://Device DISCRIPTOR
					usb_get_device_descriptor(req_data);break;
		case CONFIGURATION_DESCRIPTOR ://Config DISCRIPTOR
					usb_get_cfg_descriptor(req_data);break;
		case STRING_DESCRIPTOR ://String DISCRIPTOR
					udp_get_dev_descriptor_string(req_data);break;
		case HID_DESCRIPTOR_TYPE ://Hid DISCRIPTOR
					usb_get_hid_descriptor(req_data);break;
		case REPORT_DESCRIPTOR ://Hid DISCRIPTOR
					usb_get_report_descriptor(req_data);break;
		default:
					usbprint("usb get descriptor : 0x%04x not suppost!\n",wValue);
					usb_device_read_data_status_ep0(1);
					break;
	}
}
static void set_address(unsigned char addr)
{
    //GD_USB_Set_FADDR((U8)wValue);

//	usb_device_read_data_status_ep0(1);
//	USBC_SelectActiveEp(0);
	usb_device_clear_setup_end();
//    //GD_USB_Set_CSR0L(0x88);                 //SET  in buffer enable  // ServicedRxPktRdy & DataEnd, by yke
//	//USBC_Writeb(0x88,USBC_REG_CSR0(USBC0_BASE));
//
//
	usb_device_set_address(addr);
	usb_device_read_data_status_ep0(1);

	//usb_device_send_nullpack_ep0();
//
    usbprint("set dev address: %x\n", addr);
    usb_device_set_ep0_state(EP0_IDLE);
    //usb_device_set_xfer_type(SET_ADDR);
   // usbprint("read usb address: %x\n", GH_USB_get_FAddr());

}

static void set_configuration(void)
{
    //SETCON_ENV = 0x01;
    //GD_USB_Set_CSR0L(0x88);   //SET  in buffer enable
    /////////
   //GD_USB_Init_msc_pipe();
    ///////////

	usb_config_ep_out(1,64,USBC_TS_TYPE_INT);
	usb_config_ep_in(2,64,USBC_TS_TYPE_INT);
	usb_device_read_data_status_ep0(1);
	usb_device_clear_setup_end();
	usb_device_send_nullpack_ep0();
	usb_device_set_ep0_state(EP0_IDLE);
}
static void standard_setup_request(USB_DeviceRequest *req_data)
{
   // usb_delay(10000);
//				printf("setupdata[1]:0x%02x\n",setupdata[1]);
//				printf("setupdata[2]:0x%02x\n",setupdata[2]);
//				printf("setupdata[3]:0x%02x\n",setupdata[3]);
//				printf("setupdata[4]:0x%02x\n",setupdata[4]);
//				printf("setupdata[5]:0x%02x\n",setupdata[5]);
//				printf("setupdata[6]:0x%02x\n",setupdata[6]);
//				printf("setupdata[7]:0x%02x\n",setupdata[7]);
    //bRequest        = setupdata[1];
	unsigned char bRequest =  req_data->bRequest;
    if(bRequest==USB_REQ_GET_DESCRIPTOR)
    {
    	//u16 test;
    	usbprint("getDescriptor\n");
    	//GD_USB_Read_Clear_IntrTx(&test);   //clear int
    	usb_descriptor_request(req_data);
    }
    else if(bRequest==USB_REQ_SET_CONFIGURATION)
    {
    	usbprint("set_configuration\n");
        set_configuration();
    }
    else if(bRequest==USB_REQ_GET_CONFIGURATION)
    {
    	usbprint("get_configuration\n");
        //not support
        usbprint("Error!! unsupprot USB_REQ_GET_CONFIGURATION command");
        usb_device_read_data_status_ep0(1);
    }
    else if(bRequest==USB_REQ_SET_INTERFACE)
    {
    	usbprint("set_interface\n");
        //not support
        usbprint("Error!! unsupprot USB_REQ_SET_INTERFACE command");
        usb_device_read_data_status_ep0(1);
    }
    else if(bRequest==USB_REQ_GET_INTERFACE)
    {
        //not support
        usbprint("Error!! unsupprot USB_REQ_GET_INTERFACE command");
        usb_device_read_data_status_ep0(1);
    }
    else if(bRequest==USB_REQ_SET_ADDRESS)
    {
    	usbprint("set_addr\n");
    	volatile int time_dly = 100000;
    	while(time_dly--);//delay wait zero out pack ok
        set_address(req_data->wValue&0x7f);
    }
    else if(bRequest==USB_REQ_SET_DESCRIPTOR)
    {
    	usbprint("set_Descriptor\n");
        //not support
        usbprint("Error!! unsupprot USB_REQ_SET_DESCRIPTOR command");
        usb_device_read_data_status_ep0(1);
    }
    else if(bRequest==USB_REQ_SYNCH_FRAME)
    {
    	usbprint("sync frame\n");
        //not support
        usbprint("Error!! unsupprot USB_REQ_SYNCH_FRAME command");
        usb_device_read_data_status_ep0(1);
    }
    else
    {
    	usbprint("other  req \n");
        usbprint("Error!! received controller command:%08X.", bRequest);
        usb_device_read_data_status_ep0(1);
        //other command process by controller
    }

}
static void class_setup_request(USB_DeviceRequest* req_data)
{
	unsigned char bRequest = req_data->bRequest;
	//u32 des_length,data_tmp[26],data_flag;
	//usbprint("[GK]Received class_setuptran command\n");
   if(bRequest==HID_REQ_SET_REPORT)//HID Set report
   {
	   usbprint("HID_REQ_SET_REPORT, host set device_report success!!!\n");
   }
   else if(bRequest==HID_REQ_SET_IDLE)
   {
	   usbprint("HID_REQ_SET_IDLE, host set device_report success!!!\n");
   }
   else if(bRequest == 0x20)//set_line_coding
   {
		//set_line_coding(handle, req_data);
	   usbprint("set_line_coding\n");
		//port_line_coding_flag = 1;
		//GD_USB_Set_CSR0L(0x88);
   }
   else if(bRequest == 0x21)//get_line_coding
	{
		//get_line_coding(handle, req_data);
		//int lent = sizeof(USB_CDC_LineCoding);
		//GD_USB_Control_in((const U8 *)(&port_line_coding), &lent, 1);
		//GD_USB_Set_CSR0L(0x88);
	   usbprint("get_line_coding\n");
	}
   else if(bRequest == 0x22)//set_line_coding
	{
		//set_control_line_state(handle, req_data);
		//GD_USB_Set_CSR0L(0x88);
	   usbprint("set_control_line_state \n");
	}
   else
   {
	   usbprint("Warnning!!! received unsupport command:%08x ,do nothing \n", bRequest);
   }
 }
void usb_hid_setup_handle(unsigned char *dat,int len)
{
	if(len == 8)
	{
		memcpy(&usb_setup_request,dat,len);//usb is big endian
//		usb_setup_request.bRequestType = dat[0];
//		usb_setup_request.bRequest = dat[1];
//		usb_setup_request.wValue = (dat[2]<<8)|dat[3];
//		usb_setup_request.wIndex = (dat[4]<<8)|dat[5];
//		usb_setup_request.wLength = (dat[6]<<8)|dat[7];
		usbprint("bRequestType:0x%02x\n",usb_setup_request.bRequestType);
		usbprint("bRequest:0x%02x\n",usb_setup_request.bRequest);
		usbprint("wValue:0x%04x\n",usb_setup_request.wValue);
		usbprint("wIndex:0x%04x\n",usb_setup_request.wIndex);
		usbprint("wLength:0x%04x\n",usb_setup_request.wLength);
		if((usb_setup_request.bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD)
		{
			standard_setup_request(&usb_setup_request);
		}
		else if((usb_setup_request.bRequestType & USB_TYPE_MASK) == USB_TYPE_CLASS)
		{
			class_setup_request(&usb_setup_request);
		}
		else if((usb_setup_request.bRequestType & USB_TYPE_MASK) == USB_TYPE_VENDOR)
		{

		}

	}
	else
	{
		usbprint("not setup data.\n");
	}
}

unsigned char keyboard_led_data;
void usb_hid_out_ep_callback(unsigned char *pdat,int len)
{
	if(len == 1)
	{
		keyboard_led_data = pdat[0];
	}
}

int usb_hid_in_ep_callback(void)
{
	//usb_hid_fill_repot_send();
}

int usb_hid_send_data(unsigned char *buf,int len)
{
	int i;
	unsigned char dat[8];
	unsigned char  flag,val;
	static unsigned char last_key = 0;
	static unsigned char  last_data = 0;
	for(i = 0 ; i < len ; i++)
	{
		if((last_data == 0x0d)&&(buf[i]==0x0a))
		{
			i++;
		}
		last_data = buf[i];
		val = EN_keyboard_value[buf[i]].value;
		//in here should check the capslock key status
		flag = EN_keyboard_value[buf[i]].flag;

		if(val == 0)
		{
			val = EN_keyboard_value['?'].value;
			flag = EN_keyboard_value['?'].flag;
		}
		dat[0]= flag;
		dat[1]= 0;
		dat[2]= val;
		dat[3]= 0;
		dat[4]= 0;
		dat[5]= 0;
		dat[6]= 0;
		dat[7]= 0;
		usb_device_write_data(2,dat,8);//
		if(last_key != val) //if not the same key ,no release
		{
			dat[0]= 0;
			dat[2]= 0;
			usb_device_write_data(2,dat,8);//
		}
	}
	return 0;
}
