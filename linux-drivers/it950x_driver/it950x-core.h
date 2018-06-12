/**
 * Copyright (c) 2016 ITE Corporation. All rights reserved. 
 *
 * Module Name:
 *   it950x-core.h
 *
 * Abstract:
 *   The definitions of header for core driver.
 *
 */

#ifndef _IT950x_H_
#define _IT950x_H_

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/usb.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>
#include "iocontrol.h"
#include "IT9133.h"
#include "IT9507.h"
#include "userdef.h"
#include "modulatorFirmware.h"
#include "modulatorType.h"
#include "Common.h"
#include "tuner.h"
#include <linux/version.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/gfp.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 39)
#include <linux/smp_lock.h>
#endif

#define   DRIVER_RELEASE_VERSION    "v16.11.10.1"

/** Device Power control for DTVCAM, AVSENDER ,OTHER **/
#define DEVICE_POWER_CTRL 1			// 0: PW Control OFF
#define RETURN_CHANNEL 0			//	Return Channel 0: OFF
#define PACKECT_SHIFT 0				// Packet Shift Issue 0: OFF

/************** URB DEFINITION *************/
#define URB_COUNT_TX					16
#define URB_COUNT_TX_LOW_BRATE		32
#define URB_COUNT_TX_CMD				50
#define URB_COUNT_RX					16
#define URB_BUFSIZE_TX				32712
#define URB_BUFSIZE_TX_LOW_BRATE	188 * 2
#define URB_BUFSIZE_TX_CMD			188
#define URB_BUFSIZE_RX				188 * 348
#define CLEAN_HARDWARE_BUFFER_SIZE	1000

/** Get a minor range for devices from the usb maintainer **/
#define USB_it950x_MINOR_RANGE 32
#define USB_it913x_MINOR_RANGE 31
#ifdef CONFIG_USB_DYNAMIC_MINORS
#define USB_it913x_MINOR_BASE	0
#define USB_it950x_MINOR_BASE	USB_it913x_MINOR_RANGE
#else
#define USB_it913x_MINOR_BASE	192
#define USB_it950x_MINOR_BASE	192 + USB_it913x_MINOR_RANGE
#endif

/** USB Handler table for register kernel. **/
#define TX_MAX_USB_HANDLE_NUM USB_it950x_MINOR_RANGE
#define RX_MAX_USB_HANDLE_NUM USB_it913x_MINOR_RANGE
#define MAX_USB_HANDLE_NAME_SIZE 20		/* Table size for Tx or Rx. */

/***************** Customization *****************/
//#define QuantaMID 1
//#define EEEPC 1

/***************** from compat.h *****************/
#if LINUX_VERSION_CODE <=  KERNEL_VERSION(2,6,18)
//typedef int bool;
#define true	1
#define false	0
#endif
/***************** from device.h *****************/
#define AFA_USB_DEVICE
#define SLAVE_DEMOD_2WIREADDR  0x3A
#define TS_PACKET_SIZE		188
#define TS_PACKET_COUNT_HI	348
#define TS_PACKET_COUNT_FU	21

/***************** from driver.h *****************/
#define TS_FRAMES_HI 128
#define TS_FRAMES_FU 128
#define MAX_USB20_IRP_NUM  5
#define MAX_USB11_IRP_NUM  2

/***************** from afdrv.h *****************/
//#define GANY_ONLY 0x42F5
#define EEPROM_FLB_OFS  8

#define EEPROM_IRMODE		(OVA_EEPROM_CFG+EEPROM_FLB_OFS+0x10)   //00:disabled, 01:HID
#define EEPROM_SELSUSPEND	(OVA_EEPROM_CFG+EEPROM_FLB_OFS+0x28)   //Selective Suspend Mode
#define EEPROM_TSMODE		(OVA_EEPROM_CFG+EEPROM_FLB_OFS+0x28+1) //0:one ts, 1:dual ts
#define EEPROM_2WIREADDR		(OVA_EEPROM_CFG+EEPROM_FLB_OFS+0x28+2) //MPEG2 2WireAddr
#define EEPROM_SUSPEND		(OVA_EEPROM_CFG+EEPROM_FLB_OFS+0x28+3) //Suspend Mode
#define EEPROM_IRTYPE		(OVA_EEPROM_CFG+EEPROM_FLB_OFS+0x28+4) //0:NEC, 1:RC6
#define EEPROM_SAWBW1		(OVA_EEPROM_CFG+EEPROM_FLB_OFS+0x28+5)
#define EEPROM_XTAL1			(OVA_EEPROM_CFG+EEPROM_FLB_OFS+0x28+6) //0:28800, 1:20480
#define EEPROM_SPECINV1		(OVA_EEPROM_CFG+EEPROM_FLB_OFS+0x28+7)
#define EEPROM_TUNERID		(OVA_EEPROM_CFG+EEPROM_FLB_OFS+0x30+4) 
#define EEPROM_IFFREQL		(OVA_EEPROM_CFG+EEPROM_FLB_OFS+0x30) 
#define EEPROM_IFFREQH		(OVA_EEPROM_CFG+EEPROM_FLB_OFS+0x30+1)   
#define EEPROM_IF1L			(OVA_EEPROM_CFG+EEPROM_FLB_OFS+0x30+2)   
#define EEPROM_IF1H			(OVA_EEPROM_CFG+EEPROM_FLB_OFS+0x30+3)
#define EEPROM_SHIFT			(0x10) //EEPROM Addr Shift for slave front end
#define EEPROM_SLAVEI2CADDRESS	(OVA_EEPROM_CFG+EEPROM_FLB_OFS+0x2A)


extern int dvb_usb_it950x_debug;

/* Driver Debug */
#define DRV_DEBUG 1
/* Ring Buffer Debug */
#define RB_DEBUG 0

#if (DRV_DEBUG || RB_DEBUG)
#define deb_data(args...)   printk(KERN_NOTICE args)
#else
#define deb_data(args...)
#endif


//***************** from device.h *****************//

/*AirHD no use, RC, after kernel 38 support*/
struct it950x_config {
	u8  dual_mode:1;
	u16 mt2060_if1[2];
	u16 firmware_size;
	u16 firmware_checksum;
	u32 eeprom_sum;
};

typedef struct _TUNER_INFO {
    Bool bTunerInited;
    Bool bSettingFreq;
    BYTE TunerId;
    Bool bTunerOK;
	Bool bTunerLock;//AirHD
    Tuner_struct MXL5005_Info;
} TUNER_INFO, *PTUNER_INFO;

typedef struct _FILTER_CONTEXT_HW {
    DWORD ulCurrentFrequency;
    WORD  ucCurrentBandWidth;  
    DWORD ulDesiredFrequency;
    WORD  ucDesiredBandWidth;   
    //ULONG ulBandWidth;   
    Bool bTimerOn;
   // PKSFILTER filter;
    Byte GraphBuilt;
    TUNER_INFO tunerinfo; 
    //SIGNAL_STATISTICS ss;
    //SIGNAL_RETRAIN sr;  
    //DWORD   gdwOrigFCW;        //move from AF901x.cpp [global variable]
    //BYTE    gucOrigUnplugTh;   //move from AF901x.cpp [global variable]
    //BYTE    gucPreShiftIdx;    //move from AF901x.cpp [global variable]    
    // PKSFILTERFACTORY  pFilterFactory;
    int  bEnPID;
    //ULONG ulcPIDs;
    //ULONG aulPIDs[32];
    Bool bApOn;
    int bResetTs;
    Byte OvrFlwChk;
} FILTER_CONTEXT_HW, *PFILTER_CONTEXT_HW;  



typedef struct _DEVICE_CONTEXT {
    FILTER_CONTEXT_HW fc[2];
    Byte DeviceNo;
    Bool bBootCode;
    Bool bEP12Error;
    Bool bEP45Error;
    //bool bDebugMsg;
    //bool bDevExist;
    Bool bDualTs;
    Bool bIrTblDownload;    
    Byte BulkOutData[256];
    u32 WriteLength;
    Bool bSurpriseRemoval;
    Bool bDevNotResp;
    Bool bEnterSuspend;
    Bool bSupportSuspend;
    Bool bSupportSelSuspend;
    u16 regIdx; 
    Byte eepromIdx;
    u16 UsbMode;
    u16 MaxPacketSize;
    u32 MaxIrpSize;
    u32 TsFrames;
    u32 TsFrameSize;
    u32 TsFrameSizeDw;
    u32 TsPacketCount;
    //BYTE  ucDemod2WireAddr;
    //USB_IDLE_CALLBACK_INFO cbinfo;          // callback info for selective suspend          // our selective suspend IRP    

    Bool    bSelectiveSuspend;
    u32   ulActiveFilter;
    //BYTE    ucSerialNo; 
    Architecture architecture;
    //BYTE Tuner_Id;
    StreamType StreamType;
    Bool bDCAPIP;
    Bool bSwapFilter;
    Byte FilterCnt;
    Bool  bTunerPowerOff;
    //PKSPIN PinSave;
    Byte UsbCtrlTimeOut;
	
	Modulator modulator;            // IT9507 Modulator struct.
	DefaultDemodulator demodulator;    // IT9133 Modulator struct.
	Byte deviceType;
	Byte isUsingDefaultDeviceType;
	Bool Rx_init_success;	
	Bool Tx_init_success;	
	Handle handle_driver;
	/* Calibration Table */
	CalibrationInfo calibrationInfo;	
	
	/* DC Calibration Info */
	DCInfo dcInfo;

    Bool ForceWrite;
    Byte chip_version;
    Bool bProprietaryIr;
    Byte bIrType;

    Byte 	*pControlBuffer_WriteReg;	/* Malloc Usb2_writeControlBus Write Buffer */
    Byte 	*pControlBuffer_ReadReg;	/* Malloc Usb2_writeControlBus Read Buffer */
	
} DEVICE_CONTEXT, *PDEVICE_CONTEXT;


struct it950x_ofdm_channel {
	u32 RF_kHz;
	u8  Bw;
	s16 nfft;
	s16 guard;
	s16 nqam;
	s16 vit_hrch;
	s16 vit_select_hp;
	s16 vit_alpha;
	s16 vit_code_rate_hp;
	s16 vit_code_rate_lp;
	u8  intlv_native;
};

struct tuner_priv {
        struct tuner_config *cfg;
        struct i2c_adapter   *i2c;

        u32 frequency;
        u32 bandwidth;
        u16 if1_freq;
        u8  fmfreq;
};

/* Structure for urb context */
typedef struct it950x_urb_context{
	struct it950x_dev *dev;
	Byte index;
} URBContext;

/* Structure to hold all of our device specific stuff */
struct it950x_dev {
	struct usb_device *	usbdev;				/* the usb device for this device */
	struct usb_interface *	interface;		/* the interface for this device */
//	unsigned char *		bulk_in_buffer;		/* the buffer to receive data */
//	size_t			bulk_in_size;		    /* the size of the receive buffer */
//	__u8			bulk_in_endpointAddr;	/* the address of the bulk in endpoint */
//	__u8			bulk_out_endpointAddr;	/* the address of the bulk out endpoint */
	struct kref		kref;
	struct file *file;
	struct file *tx_file;
	DEVICE_CONTEXT DC;	
	Byte tx_on;
	Byte rx_on;
	Byte rx_chip_minor;
	Byte tx_chip_minor;	
	short tx_handle_num;
	short rx_handle_num;	
	
	Byte g_AP_use;
	atomic_t g_AP_use_tx;
	atomic_t g_AP_use_rx;
	atomic_t tx_pw_on;	
	atomic_t rx_pw_on;	
	Bool DeviceReboot, DevicePower;	
	Bool TunerInited0, TunerInited1;	
	Byte is_use_low_brate;
	int urb_counter_low_brate;
	
	/* USB URB Related for TX*/
	int	   tx_urb_streaming;
	struct urb *tx_urbs[URB_COUNT_TX];
	URBContext tx_urb_context[URB_COUNT_TX];
	Byte tx_urbstatus[URB_COUNT_TX];
	Byte tx_urb_index;
	int tx_urb_counter;
	spinlock_t TxRBKeyLock;
	Byte* pTxRingBuffer;				// Tx Ring Buffer Address.
	Dword TxCurrBuffPointAddr;		// Output urb addr.
	Dword TxWriteBuffPointAddr;		// Entry of Ring Buffer.
	Dword dwTxWriteTolBufferSize;	// Total ringbuffer size.
	Dword dwTxRemaingBufferSize;	// Remaining size in buffer.

	/* USB URB Related for TX low bitrate*/
	int	   tx_urb_streaming_low_brate;
	struct urb *tx_urbs_low_brate[URB_COUNT_TX_LOW_BRATE];
	URBContext tx_urb_context_low_brate[URB_COUNT_TX_LOW_BRATE];
	Byte tx_urbstatus_low_brate[URB_COUNT_TX_LOW_BRATE];
	Byte tx_urb_index_low_brate;
	int tx_urb_counter_low_brate;
	spinlock_t TxRBKeyLock_low_brate;
	Byte* pTxRingBuffer_low_brate;
	//Byte* pWriteFrameBuffer_low_brate;
	Dword TxCurrBuffPointAddr_low_brate;	
	Dword TxWriteBuffPointAddr_low_brate;	
	Dword dwTxWriteTolBufferSize_low_brate;
	Dword dwTxRemaingBufferSize_low_brate;	
#if RETURN_CHANNEL
	/* USB URB Related for TX_CMD */
	int	   tx_urb_streaming_cmd;
	struct urb *tx_urbs_cmd[URB_COUNT_TX_CMD];
	URBContext tx_urb_context_cmd[URB_COUNT_TX_CMD];
	Byte tx_urbstatus_cmd[URB_COUNT_TX_CMD];
	Byte tx_urb_index_cmd;
	//Byte urb_use_count_cmd;
	spinlock_t TxRBKeyLock_cmd;		
	Byte* pTxRingBuffer_cmd;
	Byte* pWriteFrameBuffer_cmd;
	Dword* pTxCurrBuffPointAddr_cmd;	
	Dword* pTxWriteBuffPointAddr_cmd;	
	Dword dwTxWriteTolBufferSize_cmd;
	Dword dwTxRemaingBufferSize_cmd;	
#endif
	/* USB URB Related for RX */
	int    rx_urb_streaming;
	struct urb *rx_urbs[URB_COUNT_RX];
	URBContext rx_urb_context[URB_COUNT_RX];
	Byte rx_urbstatus[URB_COUNT_RX];
	atomic_t rx_urb_counter;
	Byte rx_urb_index;
	Byte rx_first_urb_reset;	
	spinlock_t RxRBKeylock;
	Byte *pRxRingBuffer;
	Dword RxCurrBuffPointAddr;
	Dword RxReadBuffPointAddr;
	Dword dwRxTolBufferSize;
	Dword dwRxReadRemaingBufferSize;

	/* Patch for packet shift */
#if PACKECT_SHIFT
	int buffer_shift;
	struct task_struct *reset_thread;
	wait_queue_head_t reset_wait;
	Byte disconnect;	
#endif
};

//extern struct usb_device *udevs;
//extern PDEVICE_CONTEXT PDC;
//extern int it950x_device_count;

extern DWORD Device_init(struct usb_device *udev,PDEVICE_CONTEXT PDCs, Bool bBoot);
extern DWORD DL_ApCtrl(void* handle, Byte ucSlaveDemod, Bool bOn);
extern DWORD DL_ApPwCtrl(void* handle, Bool ucSlaveDemod, Bool bOn);
extern DWORD DL_Tuner_SetFreqBw(void *handle, BYTE ucSlaveDemod,u32 ucFreq,u8 ucBw);
extern DWORD DL_isLocked(void *handle,BYTE ucSlaveDemod, Bool *bLock);
extern DWORD DL_getSignalStrength(void *handle,BYTE ucSlaveDemod, BYTE* strength);
extern DWORD DL_getSignalStrengthDbm(void *handle, BYTE ucSlaveDemod, Long* strengthDbm);
extern DWORD DL_getChannelStatistic(BYTE ucSlaveDemod, ChannelStatistic*  channelStatistic);
extern DWORD DL_getChannelModulation(void *handle, BYTE ucSlaveDemod, ChannelModulation*    channelModulation);
extern DWORD DL_getSNR(void *handle, BYTE ucSlaveDemod, Constellation* constellation, BYTE* snr);
extern DWORD DL_ReSetInterval(void);
extern DWORD DL_Reboot(void* handle);
extern DWORD DL_CheckTunerInited(void *handle, BYTE ucSlaveDemod, Bool *bOn);
extern DWORD DL_DemodIOCTLFun(void* handle, DWORD IOCTLCode, unsigned long pIOBuffer);
extern DWORD DL_LoadIQtable_Fromfile(void *handle);
extern DWORD DL_ResetBuffer(void *handle);
extern DWORD DL_SetDCCalibrationTable(void *handle);
//extern Bool DevicePower, DeviceReboot;
// TunerInited0, TunerInited1
//extern DWORD DL_NIMSuspend(void* handle, bool bSuspend);
//extern DWORD DL_SetArchitecture(void* handle, Architecture architecture);
//extern DWORD DL_ApReset (void * handle, BYTE ucSlaveDemod, Bool bOn);
//extern DWORD DL_WriteRegisters (void * handle, BYTE ucSlaveDemod, Processor	processor, DWORD wReg, BYTE ucpValueLength, BYTE* ucpValue);
//extern DWORD DL_ReadRegisters (void * handle, BYTE ucSlaveDemod, Processor	processor, DWORD wReg, BYTE ucpValueLength, BYTE* ucpValue);
//extern void it9507_set_remote_config(struct usb_device *udev, struct dvb_usb_device_properties *props);

#endif

