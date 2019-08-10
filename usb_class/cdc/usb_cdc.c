#include "usb.h"
#include "usb_dev.h"
extern int ep_max_len[];
/*
 * USB types, the second of three bRequestType fields
 */
#define USB_TYPE_MASK                   (0x03 << 5)
#define USB_TYPE_STANDARD               (0x00 << 5)
#define USB_TYPE_CLASS                  (0x01 << 5)
#define USB_TYPE_VENDOR                 (0x02 << 5)
#define USB_TYPE_RESERVED               (0x03 << 5)



static USB_DeviceRequest usb_setup_request;

static USB_DeviceDescriptor cdc_devDesc =
{
	sizeof(USB_DeviceDescriptor),
	DEVICE_DESCRIPTOR,	//1
    #if DWC_FORCE_SPEED_FULL
	0x0111,     //Version 1.11
	#else
    0x0200,     //Version 2.0
    #endif
	0x02,
	0x00,
	0x00,
	64,	/* Ep0 FIFO size */
	0x0483,
	0x5740,
	USB_FUNCTION_RELEASE_NO,
	0x01, 	//iManufacturer;
	0x02,   //iProduct;
	0x00,
	0x01
};

static USB_DeviceDescriptor cdc_devFullSpeedDesc =
{
	sizeof(USB_DeviceDescriptor),
	DEVICE_DESCRIPTOR,	//1
	0x0111,     //Version 1.11
	0x02,
	0x00,
	0x00,
	64,	/* Ep0 FIFO size */
	0x0483,
	0x5740,
	USB_FUNCTION_RELEASE_NO+1,
	0x01, 	//iManufacturer;
	0x02,   //iProduct;
	0x00,
	0x01
};

#define	CONFIG_CDC_DESCRIPTOR_LEN	(sizeof(USB_ConfigDescriptor) + \
				 sizeof(USB_InterfaceAssocDescriptor) + \
				 sizeof(USB_InterfaceDescriptor) + \
				 sizeof(USB_CDC_HeaderDescriptor) + \
				 sizeof(USB_CDC_CallMgmtDescriptor) + \
				 sizeof(USB_CDC_AcmDescriptor) + \
				 sizeof(USB_CDC_UnionDescriptor) + \
				 sizeof(USB_EndPointDescriptor) + \
				 sizeof(USB_InterfaceDescriptor) + \
				 sizeof(USB_EndPointDescriptor) * 2)


static struct {
	USB_ConfigDescriptor    configuration_descriptor;
	USB_InterfaceAssocDescriptor    interface_assoc_descriptor;
	USB_InterfaceDescriptor control_interface_descritor;
	USB_CDC_HeaderDescriptor header_descriptor;
	USB_CDC_CallMgmtDescriptor call_mgmt_descriptor;
	USB_CDC_AcmDescriptor acm_descriptor;
	USB_CDC_UnionDescriptor union_descriptor;
	USB_EndPointDescriptor  hs_notify_descriptor;
	USB_InterfaceDescriptor data_interface_descritor;
	USB_EndPointDescriptor  endpoint_descriptor[2];
} __attribute__ ((packed)) cdc_confDesc = {
	{
		sizeof(USB_ConfigDescriptor),
		CONFIGURATION_DESCRIPTOR,
		CONFIG_CDC_DESCRIPTOR_LEN,  /*  Total length of the Configuration descriptor */
		0x02,                   /*  NumInterfaces */
		0x02,                   /*  Configuration Value */
		0x04,                   /* Configuration Description String Index */
		0xc0,	// Self Powered, no remote wakeup
		0x32	// Maximum power consumption 500 mA
	},
	{
		sizeof(USB_InterfaceAssocDescriptor),
		INTERFACE_ASSOC_DESCRIPTOR,
		0x00,
		0x02,
		0x02,
		0x02,
		0x01,
		0x00
	},
	{
		sizeof(USB_InterfaceDescriptor),
		INTERFACE_DESCRIPTOR,
		0x00,   /* bInterfaceNumber */
		0x00,   /* bAlternateSetting */
		0x01,	/* ep number */
		0x02,   /* Interface Class */
		0x02,      /* Interface Subclass*/
		0x01,      /* Interface Protocol*/
		0x05    /* Interface Description String Index*/
	},
	{
		sizeof(USB_CDC_HeaderDescriptor),
		USB_DT_CS_INTERFACE,
		0x00,
		0x0110
	},
	{
		sizeof(USB_CDC_CallMgmtDescriptor),
		USB_DT_CS_INTERFACE,
		0x01,
		0x00,
		0x01
	},
	{
		sizeof(USB_CDC_AcmDescriptor),
		USB_DT_CS_INTERFACE,
		0x02,
		0x02
	},
	{
		sizeof(USB_CDC_UnionDescriptor),
		USB_DT_CS_INTERFACE,
		0x06,
		0x00,
		0x01
	},
	{
		sizeof(USB_EndPointDescriptor),
		ENDPOINT_DESCRIPTOR,
		(1 << 7) | 2,	/* endpoint 2 IN */
		3,	/* interrupt */
		10,	/* IN EP FIFO size */
		9	/* Interval */
	},
	{
		sizeof(USB_InterfaceDescriptor),
		INTERFACE_DESCRIPTOR,
		0x01,
		0x00,
		0x02,
		0x0a,
		0x00,
		0x00,
		0x06
	},
	{
		{
			sizeof(USB_EndPointDescriptor),
			ENDPOINT_DESCRIPTOR,
			(1 << 7) | 1,// endpoint 1 IN
			2, /* bulk */
			/* Transfer Type: Bulk;
			 * Synchronization Type: No Synchronization;
			 * Usage Type: Data endpoint
			 */
           	512,
			0
		},
		{
			sizeof(USB_EndPointDescriptor),
			ENDPOINT_DESCRIPTOR,
			(0 << 7) | 1,// endpoint 1 OUT
			2, /* bulk */
			/* Transfer Type: Bulk;
			 * Synchronization Type: No Synchronization;
			 * Usage Type: Data endpoint
			 */
           	512,
			0
		}
	}
},

cdc_confFullSpeedDesc = {
	{
		sizeof(USB_ConfigDescriptor),
		CONFIGURATION_DESCRIPTOR,
		CONFIG_CDC_DESCRIPTOR_LEN,  /*  Total length of the Configuration descriptor */
		0x02,                   /*  NumInterfaces */
		0x02,                   /*  Configuration Value */
		0x04,                   /* Configuration Description String Index */
		0xc0,	// Self Powered, no remote wakeup
		0x32	// Maximum power consumption 500 mA
	},
	{
		sizeof(USB_InterfaceAssocDescriptor),
		INTERFACE_ASSOC_DESCRIPTOR,
		0x00,
		0x02,
		0x02,
		0x02,
		0x01,
		0x07
	},
	{
		sizeof(USB_InterfaceDescriptor),
		INTERFACE_DESCRIPTOR,
		0x00,   /* bInterfaceNumber */
		0x00,   /* bAlternateSetting */
		0x01,	/* ep number */
		0x02,   /* Interface Class */
		0x02,      /* Interface Subclass*/
		0x01,      /* Interface Protocol*/
		0x05    /* Interface Description String Index*/
	},
	{
		sizeof(USB_CDC_HeaderDescriptor),
		USB_DT_CS_INTERFACE,
		0x00,
		0x0110
	},
	{
		sizeof(USB_CDC_CallMgmtDescriptor),
		USB_DT_CS_INTERFACE,
		0x01,
		0x00,
		0x01
	},
	{
		sizeof(USB_CDC_AcmDescriptor),
		USB_DT_CS_INTERFACE,
		0x02,
		0x02
	},
	{
		sizeof(USB_CDC_UnionDescriptor),
		USB_DT_CS_INTERFACE,
		0x06,
		0x00,
		0x01
	},
	{
		sizeof(USB_EndPointDescriptor),
		ENDPOINT_DESCRIPTOR,
		(1 << 7) | 2,	/* endpoint 2 IN */
		3,	/* interrupt */
		10,	/* IN EP FIFO size */
		9	/* Interval */
	},
	{
		sizeof(USB_InterfaceDescriptor),
		INTERFACE_DESCRIPTOR,
		0x01,
		0x00,
		0x02,
		0x0a,
		0x00,
		0x00,
		0x06
	},
	{
		{
			sizeof(USB_EndPointDescriptor),
			ENDPOINT_DESCRIPTOR,
			(1 << 7) | 1,// endpoint 1 IN
			2, /* bulk */
			/* Transfer Type: Bulk;
			 * Synchronization Type: No Synchronization;
			 * Usage Type: Data endpoint
			 */
			64, /* IN EP FIFO size */
			0
		},
		{
			sizeof(USB_EndPointDescriptor),
			ENDPOINT_DESCRIPTOR,
			(0 << 7) | 1,// endpoint 1 OUT
			2, /* bulk */
			/* Transfer Type: Bulk;
			 * Synchronization Type: No Synchronization;
			 * Usage Type: Data endpoint
			 */
			64, /* OUT EP FIFO size */
			0
		}
	}
};

static unsigned short str_ret[] = {
		0x0326,
		'E','a','s','y',' ','U','S','B',' ','-',' ','D','e','v','i','c','e','s'
};

static unsigned short str_lang[] = {
	0x0304,
	0x0409
};

static unsigned short str_isernum[] = {
		0x033e,
		'E','a','s','y',' ','U','S','B',' ','-',' ','V','i','r','t','r','u','e',' ','C','O','M',' ','D','e','v','i','c','e',' '
};

static unsigned short str_config[] = {
		0x031e,
		'C','D','C',' ','A','C','M',' ','c','o','n','f','i','g'
};

static unsigned short str_interface1[] = {
		0x0342,
		'C','D','C',' ','A','b','s','t','r','a','c','t',' ','C','o','n','t','r','o','l',' ','M','o','d','e','l',' ','(','A','C','M',')'
};

static unsigned short str_interface2[] = {
		0x031a,
		'C','D','C',' ','A','C','M',' ','D','a','t','a'
};
USB_CDC_LineCoding port_line_coding = {
		.dwDTERate = 115200,
		.bCharFormat = 0,
		.bParityType = 0,
		.bDataBits = 8
};
unsigned int port_line_coding_flag;
void set_line_codingstatic_ext(unsigned char * pdata)
{
			port_line_coding_flag = 0;
			port_line_coding.dwDTERate = *(volatile unsigned int *)(pdata);
			port_line_coding.bCharFormat = *(volatile unsigned char *)(pdata+4);
			port_line_coding.bParityType = *(volatile unsigned char *)(pdata+5);
			port_line_coding.bDataBits = *(volatile unsigned char *)(pdata+6);
}
static void usb_get_device_descriptor(USB_DeviceRequest *req_data)
{
    unsigned int len = req_data->wLength;
    usbprint("usb_get_device_descriptor\n");
	if (len < sizeof(USB_DeviceDescriptor))
	{
		usb_device_write_data_ep_pack(0, (unsigned char *)&cdc_devDesc, len);
	}
	else
	{
		usb_device_write_data_ep_pack(0, (unsigned char *)&cdc_devDesc, sizeof(USB_DeviceDescriptor));
	}
}

static void usb_get_cfg_descriptor(USB_DeviceRequest *req_data)
{
	usbprint("usb_get_cfg_descriptor\n");
	switch(req_data->wLength)
	{
		case 9:
			usb_device_write_data_ep_pack(0,  (unsigned char *)&cdc_confDesc, 9);
			break;
		case 8:
			usb_device_write_data_ep_pack(0,  (unsigned char *)&cdc_confDesc, 8);
			break;
		default:
			usb_device_write_data_ep_pack(0,  (unsigned char *)&cdc_confDesc, sizeof(cdc_confDesc));
			break;
	}
}

static inline void udp_get_dev_descriptor_string(USB_DeviceRequest *dreq)
{
	int len = dreq->wLength;
	int index = dreq->wValue & 0xff;
	usbprint("udp_get_dev_descriptor_string:%d\n",index);
	switch ( index)
	{
		case 0:       //land ids
			if ( len > sizeof(str_lang) )
				usb_device_write_data_ep_pack(0,(unsigned char *)str_lang,sizeof(str_lang));
			else
				usb_device_write_data_ep_pack(0,(unsigned char *)str_lang,len);
			return;
			break;
		case 1:       //iserialnumber
			if (len >= sizeof(str_isernum))
				len = sizeof(str_isernum);
			usb_device_write_data_ep_pack(0,(unsigned char *)str_isernum,len);
			break;

		case 2:       //iproduct
			if (len >= 38)
				len = 38;
			str_ret[0] = (0x0300 | len);
			usb_device_write_data_ep_pack(0,(unsigned char *)str_ret,len);
			break;
		case 3:       //iserialnumber
			if (len >= sizeof(str_isernum))
				len = sizeof(str_isernum);
			usb_device_write_data_ep_pack(0,(unsigned char *)str_isernum,len);
			break;
		case 4:
			if (len >= sizeof(str_config))
				len = sizeof(str_config);
			usb_device_write_data_ep_pack(0,(unsigned char *)str_config,len);
			break;
		case 5:
			if (len >= sizeof(str_interface1))
				len = sizeof(str_interface1);
			usb_device_write_data_ep_pack(0,(unsigned char *)str_interface1,len);
			break;
		case 6:
			if (len >= sizeof(str_interface2))
				len = sizeof(str_interface2);
			usb_device_write_data_ep_pack(0,(unsigned char *)str_interface2,len);
			break;
		case 0xee:    //microsoft OS!
			if (len >= sizeof(str_isernum))
				len = sizeof(str_isernum);
			str_isernum[0] = (0x0300 | len);
			usb_device_write_data_ep_pack(0,(unsigned char *)str_isernum,len);
			break;
		default:
			if (len >= sizeof(str_isernum))
				len = sizeof(str_isernum);
			str_isernum[0] = (0x0300 | len);
			usb_device_write_data_ep_pack(0,(unsigned char *)str_isernum,len);
			break;
	}
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
//		case HID_DESCRIPTOR_TYPE ://Hid DISCRIPTOR
//					usb_get_hid_descriptor(req_data);break;
//		case REPORT_DESCRIPTOR ://Hid DISCRIPTOR
//					usb_get_report_descriptor(req_data);break;
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
//	usb_device_clear_setup_end();
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

}

static void set_configuration(void)
{
    //SETCON_ENV = 0x01;
    //GD_USB_Set_CSR0L(0x88);   //SET  in buffer enable
    /////////
   //GD_USB_Init_msc_pipe();
    ///////////

	usb_config_ep_out(1,512,USBC_TS_TYPE_BULK);
	usb_config_ep_in(1,512,USBC_TS_TYPE_BULK);
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
    	u16 test;
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
//    	volatile int time_dly = 100000;
//    	while(time_dly--);//delay wait zero out pack ok
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
    else if(bRequest == USB_REQ_CLEAR_FEATURE)
    {
    	int epaddr = req_data->wIndex;
    	usbprint("clear feature\n");
		//not support
		//usbprint("Error!! unsupprot USB_REQ_CLEAR_FEATURE command");

    	usb_device_clear_ep_halt(epaddr);
		usb_device_read_data_status_ep0(1);
    }
    else
    {
    	usbprint("other  req \n");
        usbprint("Error!! received controller command:%02X.\n", bRequest);
        usb_device_read_data_status_ep0(1);
        //other command process by controller
    }

}
static void class_setup_request(USB_DeviceRequest* req_data)
{
	unsigned char bRequest = req_data->bRequest;
	//u32 des_length,data_tmp[26],data_flag;
	if(bRequest == 0x20)//set_line_coding
   {
		//set_line_coding(handle, req_data);
	   //usb_device_read_data_status_ep0(0);
	   usbprint("set_line_coding\n");
	   port_line_coding_flag = 1;
	   int len = USBC_ReadLenFromFifo(1);
	   usbprint("set_line_coding:readlen:%d\n",len);
	   if(len == 7)
	   {
		   unsigned char tmp[7];
		   usb_device_read_data_ep_pack(0,tmp,7);
		   set_line_codingstatic_ext(tmp);
		   usb_device_read_data_status_ep0(1);
		   usb_device_set_ep0_state(EP0_IDLE);
		   port_line_coding_flag = 0;
	   }

		//GD_USB_Set_CSR0L(0x88);
   }
   else if(bRequest == 0x21)//get_line_coding
	{
		//get_line_coding(req_data);
		int lent = sizeof(USB_CDC_LineCoding);
		//usb_device_read_data_status_ep0(1);
		usb_device_write_data_ep_pack(0,(const u8 *)(&port_line_coding),lent);
		//GD_USB_Set_CSR0L(0x88);
	   usbprint("get_line_coding\n");
	}
   else if(bRequest == 0x22)//set_control_line_state
	{
		//set_control_line_state(handle, req_data);
		//GD_USB_Set_CSR0L(0x88);
	   usbprint("set_control_line_state \n");
	   //usb_device_read_data_status_ep0(1);
	   usb_device_clear_setup_end();
		//usb_device_send_nullpack_ep0();
	   usb_device_set_ep0_state(EP0_IDLE);

	}
   else
   {
	   usbprint("Warnning!!! received unsupport command:%08x ,do nothing\n", bRequest);
   }
 }
void usb_cdc_setup_handle(unsigned char *dat,int len)
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

int usb_cdc_send_data(unsigned char *buf,int len)
{
    int ret = usb_device_write_data(1, buf, len);
    if (ret != 0) {
        return ret;
    }

    // 长度整除packet_length时多发一个空packet，CDC协议要求的
    if (len % ep_max_len[1] == 0) {
        ret = usb_device_write_data(1, buf, 0);
    }
    return ret;
}
void usb_cdc_out_ep_callback(unsigned char *pdat,int len)
{
	usb_device_write_data(1,pdat,len);
}
