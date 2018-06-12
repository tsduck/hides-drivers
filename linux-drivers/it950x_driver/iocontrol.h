/**
 *
 * Copyright (c) 2016 ITE Corporation. All rights reserved. 
 *
 * Module Name:
 *   iocontrol.h
 *
 * Abstract:
 *   The structure and IO code for IO control call.
 *
 */

#ifndef __IOCONTROL_H__
#define __IOCONTROL_H__

#include "modulatorType.h"


/* Use 'k' as magic number */
#define AFA_IOC_MAGIC  'k'


typedef struct {
    Byte			chip;
    Processor		processor;
    __u32			registerAddress;
    Byte			bufferLength;
    Byte			buffer[256];
    __u32			error;
    Byte			reserved[16];
} WriteRegistersRequest, *PWriteRegistersRequest;

typedef struct {
    Byte			chip;
    Processor		processor;
    __u32			registerAddress;
    Byte			bufferLength;
    Byte			buffer[256];
    __u32			error;
    Byte			reserved[16];
} TxWriteRegistersRequest, *PTxWriteRegistersRequest;

typedef struct {
    Byte			chip;
    Word			registerAddress;
    Byte			bufferLength;
    Byte			buffer[256];
    __u32			error;
    Byte			reserved[16];
} TxWriteEepromValuesRequest, *PTxWriteEepromValuesRequest;

typedef struct {
    Byte			chip;
    Processor		processor;
    __u32			registerAddress;
    Byte			bufferLength;
    Byte			buffer[256];
    __u32			error;
    Byte			reserved[16];
} ReadRegistersRequest, *PReadRegistersRequest;

typedef struct {
    Byte			chip;
    Processor		processor;
    __u32			registerAddress;
    Byte			bufferLength;
    Byte			buffer[256];
    __u32			error;
    Byte			reserved[16];
} TxReadRegistersRequest, *PTxReadRegistersRequest;

typedef struct {
    Byte			chip;
    Word			registerAddress;
    Byte			bufferLength;
    Byte			buffer[256];
    __u32			error;
    Byte			reserved[16];
} TxReadEepromValuesRequest, *PTxReadEepromValuesRequest;

typedef struct {
    Byte				chip;
    Word				bandwidth;
    __u32				frequency;
    __u32				error;
    Byte				reserved[16];
} AcquireChannelRequest, *PAcquireChannelRequest;

typedef struct {
    Byte				chip;
    Byte 				transmissionMode;
	Byte				constellation;
	Byte				interval;
	Byte				highCodeRate;
    __u32				error;
    Byte				reserved[16];
} TxSetModuleRequest, *PTxSetModuleRequest;

typedef struct {
    Byte				chip;
    Word				bandwidth;
    __u32				frequency;
    __u32				error;
    Byte				reserved[16];
} TxAcquireChannelRequest, *PTxAcquireChannelRequest;

typedef struct {
    Byte				OnOff;
    __u32				error;
    Byte				reserved[16];
} TxModeRequest, *PTxModeRequest;

typedef struct {
    Byte				DeviceType;
    __u32				error;
    Byte				reserved[16];
} TxSetDeviceTypeRequest, *PTxSetDeviceTypeRequest;

typedef struct {
    Byte				DeviceType;
    __u32				error;
    Byte				reserved[16];
} TxGetDeviceTypeRequest, *PTxGetDeviceTypeRequest;

typedef struct {
    int				GainValue;
    __u32			error;
} TxSetGainRequest, *PTxSetGainRequest;

typedef struct {
    Byte				chip;
    Bool				locked;
    Dword				error;
    Byte				reserved[16];
} IsLockedRequest, *PIsLockedRequest;

typedef struct {
    Byte*				platformLength;
    Platform*			platforms;
    Dword				error;
    Byte				reserved[16];
} AcquirePlatformRequest, *PAcquirePlatformRequest;

typedef struct {
	Byte				chip;
	Byte				index;
    Pid					pid;
    __u32				error;
    Byte				reserved[16];
} AddPidAtRequest, *PAddPidAtRequest;

typedef struct {
	Byte				chip;
	Byte				index;
    Pid					pid;
    __u32				error;
    Byte				reserved[16];
} TxAddPidAtRequest, *PTxAddPidAtRequest;

typedef struct {
	Byte			chip;
    __u32			error;
    Byte			reserved[16];
} ResetPidRequest, *PResetPidRequest;

typedef struct {
	Byte			chip;
    __u32			error;
    Byte			reserved[16];
} TxResetPidRequest, *PTxResetPidRequest;

typedef struct {
    Byte				chip;
    __u32				channelStatisticAddr;		// ChannelStatistic*
    __u32				error;
    Byte				reserved[16];
} GetChannelStatisticRequest, *PGetChannelStatisticRequest;

typedef struct {
    Byte				chip;
	Statistic			statistic;
    __u32				error;
    Byte				reserved[16];
} GetStatisticRequest, *PGetStatisticRequest;

typedef struct {
    Byte			chip;
	Byte			control;
    __u32			error;
    Byte			reserved[16];
} ControlPidFilterRequest, *PControlPidFilterRequest;

typedef struct {
    Byte			control;
	Byte			enable;
    __u32			error;
    Byte			reserved[16];
} TxControlPidFilterRequest, *PTxControlPidFilterRequest;

typedef struct {
    Byte				chip;
    Byte				control;
    __u32				error;
    Byte				reserved[16];
} ControlPowerSavingRequest, *PControlPowerSavingRequest;

typedef struct {
    Byte				chip;
    Byte				control;
    __u32				error;
    Byte				reserved[16];
} TxControlPowerSavingRequest, *PTxControlPowerSavingRequest;

typedef struct {
    Byte                DriverVerion[16];   /** XX.XX.XX.XX Ex., 1.2.3.4			*/
    Byte                APIVerion[32];      /** XX.XX.XXXXXXXX.XX Ex., 1.2.3.4	*/
    Byte                FWVerionLink[16];   /** XX.XX.XX.XX Ex., 1.2.3.4			*/
    Byte                FWVerionOFDM[16];   /** XX.XX.XX.XX Ex., 1.2.3.4			*/
    Byte                DateTime[24];       /** Ex.,"2004-12-20 18:30:00" or "DEC 20 2004 10:22:10" with compiler __DATE__ and __TIME__  definitions */
    Byte                Company[8];         /** Ex.,"ITEtech"					*/
    Byte                SupportHWInfo[32];  /** Ex.,"Jupiter DVBT/DVBH"			*/
    __u32               error;
    Byte                reserved[128];
} DemodDriverInfo, *PDemodDriverInfo;

typedef struct {
    Byte                DriverVerion[16];   /** XX.XX.XX.XX Ex., 1.2.3.4			*/
    Byte                APIVerion[32];      /** XX.XX.XXXXXXXX.XX Ex., 1.2.3.4	*/
    Byte                FWVerionLink[16];   /** XX.XX.XX.XX Ex., 1.2.3.4			*/
    Byte                FWVerionOFDM[16];   /** XX.XX.XX.XX Ex., 1.2.3.4			*/
    Byte                DateTime[24];       /** Ex.,"2004-12-20 18:30:00" or "DEC 20 2004 10:22:10" with compiler __DATE__ and __TIME__  definitions */
    Byte                Company[8];         /** Ex.,"ITEtech"					*/
    Byte                SupportHWInfo[32];  /** Ex.,"Jupiter DVBT/DVBH"			*/
    __u32               error;
    Byte                reserved[128];
} TxModDriverInfo, *PTxModDriverInfo;

/**
 * Demodulator Stream control API commands
 */
typedef struct {
	Byte			chip;
    Dword			error;
    Byte			reserved[16];
} StartCaptureRequest, *PStartCaptureRequest;

typedef struct {
	Byte			chip;
    Dword			error;
    Byte			reserved[16];
} TxStartTransferRequest, *PTxStartTransferRequest;

typedef struct {
	Byte			chip;
    Dword			error;
    Byte			reserved[16];
} TxStopTransferRequest, *PTxStopTransferRequest;

typedef struct {
	Byte			chip;
    Dword			error;
    Byte			reserved[16];
} StopCaptureRequest, *PStopCaptureRequest;

typedef struct {
	__u32			len;
    __u32			cmdAddr;		// Byte*
    __u32			error;    
    Byte			reserved[16];
} TxCmdRequest, *PTxCmdRequest;

typedef struct {
    __u32			error;
	__u32          frequency;
	Word           bandwidth;    
	int				maxGain;
	int				minGain;
    Byte			reserved[16];	
} TxGetGainRangeRequest, *PTxGetGainRangeRequest;

typedef struct {
    TPS				tps;
    __u32			error;
    Byte			reserved[16];	
} TxGetTPSRequest, *PTxGetTPSRequest;

typedef struct {
    TPS            tps;
	Bool 		   actualInfo;    
    __u32		   error;
    Byte		   reserved[16];	
} TxSetTPSRequest, *PTxSetTPSRequest;

typedef struct {
	int			   gain;	 
    __u32		   error;
    Byte		   reserved[16];	
} TxGetOutputGainRequest, *PTxGetOutputGainRequest;

typedef struct {
    __u32		   error;
	__u32         pbufferAddr;
    Byte		   reserved[16];	
} TxSendHwPSITableRequest, *PTxSendHwPSITableRequest;

typedef struct {
	Byte		   psiTableIndex;
	__u32     	   pbufferAddr;
    __u32		   error;
    Byte		   reserved[16];	
} TxAccessFwPSITableRequest, *PTxAccessFwPSITableRequest;

typedef struct {
	Byte			psiTableIndex;
	Word			timer;
    __u32			error;
    Byte			reserved[16];	
} TxSetFwPSITableTimerRequest, *PTxSetFwPSITableTimerRequest;

typedef struct {
    __u32 				pBufferAddr;			// Byte*
    __u32 				pdwBufferLength;
    __u32				error;
    Byte				reserved[16];
} TxSetLowBitRateTransferRequest, *PTxSetLowBitRateTransferRequest;

typedef struct {
	__u32				pIQtableAddr;		// Byte*
	Word				IQtableSize;
    __u32				error;
    Byte				reserved[16];
} TxSetIQTableRequest, *PTxSetIQTableRequest;

typedef struct {
    int					dc_i;
	int					dc_q;
    __u32				error;
    Byte				reserved[16];
} TxSetDCCalibrationValueRequest, *PTxSetDCCalibrationValueRequest;

typedef struct {
    Word			chipType;	
    __u32			error;
    Byte			reserved[16];
} TxGetChipTypeRequest, *PTxGetChipTypeRequest;

typedef struct {
    __u32			  	isdbtModulationAddr;	//	ISDBTModulation
    __u32				error;
    Byte				reserved[16];
} TXSetISDBTChannelModulationRequest, *PTXSetISDBTChannelModulationRequest;

typedef struct {
    TMCCINFO      		TmccInfo;
    Bool			    actualInfo;
    __u32				error;
    Byte				reserved[16];
} TXSetTMCCInfoRequest, *PTXSetTMCCInfoRequest;

typedef struct {
    TMCCINFO          	TmccInfo;
    __u32				error;
    Byte				reserved[16];
} TXGetTMCCInfoRequest, *PTXGetTMCCInfoRequest;

typedef struct {
    Word				BitRate_Kbps;
    __u32				error;
    Byte				reserved[16];
} TXGetTSinputBitRateRequest, *PTXGetTSinputBitRateRequest;

typedef struct {
    Byte            	index;
    Pid             	pid;
	TransportLayer  	layer;
    __u32				error;
    Byte				reserved[16];
} TXAddPidToISDBTPidFilterRequest, *PTXAddPidToISDBTPidFilterRequest;

typedef struct {
    PcrMode			mode;
    __u32				error;
    Byte				reserved[16];
} TxSetPcrModeRequest, *PTxSetPcrModeRequest;

typedef struct {
	__u32				DCInfoAddr;	//DCInfo*
    __u32				error;
    Byte				reserved[16];
} TxSetDCTableRequest, *PTxSetDCTableRequest;

typedef struct {
    Byte				frequencyindex;
    __u32				error;
    Byte				reserved[16];    
} TxGetFrequencyIndexRequest, *PTxGetFrequencyIndexRequest;

typedef struct {
    Byte			DTVMode;
    __u32			error;
    Byte			reserved[16];
} TxGetDTVModeRequest, *PTxGetDTVModeRequest;

typedef struct {
    __u32			key	;
    __u32			error;
    Byte			reserved[16];
} TxEnableTpsEncryptionRequest, *PTxEnableTpsEncryptionRequest;

typedef struct {
    __u32			error;
    Byte			reserved[16];
} TxDisableTpsEncryptionRequest, *PTxDisableTpsEncryptionRequest;

typedef struct {
    __u32			decryptKey;
    Byte			decryptEnable;
    __u32			error;
    Byte			reserved[16];
} TxSetDecryptRequest, *PTxSetDecryptRequest;

typedef struct {
    Bool			isInversion;
    __u32			error;
    Byte			reserved[16];
} TxSetSpectralInversionRequest, *PTxSetSpectralInversionRequest;

/**
 * Modulator & Demodulator API commands
 */
#define IOCTRL_ITE_GROUP_STANDARD			0x000
#define IOCTRL_ITE_GROUP_DVBT				0x100
#define IOCTRL_ITE_GROUP_DVBH				0x200
#define IOCTRL_ITE_GROUP_FM					0x300
#define IOCTRL_ITE_GROUP_TDMB				0x400
#define IOCTRL_ITE_GROUP_OTHER				0x500
#define IOCTRL_ITE_GROUP_ISDBT				0x600
#define IOCTRL_ITE_GROUP_SECURITY			0x700


/***************************************************************************/
/*                             STANDARD                           */
/***************************************************************************/
/**
 * Write a sequence of bytes to the contiguous registers in demodulator.
 * Paramters:   WriteRegistersRequest struct
 */
#define IOCTL_ITE_DEMOD_WRITEREGISTERS \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x00, WriteRegistersRequest)

/**
 * Read a sequence of bytes from the contiguous registers in demodulator.
 * Paramters:   ReadRegistersRequest struct
 */
#define IOCTL_ITE_DEMOD_READREGISTERS \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x06, ReadRegistersRequest)

/**
 * Specify the bandwidth of channel and tune the channel to the specific
 * frequency. Afterwards, host could use output parameter dvbH to determine
 * if there is a DVB-H signal.
 * In DVB-T mode, after calling this function output parameter dvbH should
 * be False and host could use output parameter "locked" to indicate if the 
 * TS is correct.
 * In DVB-H mode, after calling this function output parameter dvbH should
 * be True and host could use Jupiter_acquirePlatorm to get platform. 
 * Paramters:   AcquireChannelRequest struct
 */
#define IOCTL_ITE_DEMOD_ACQUIRECHANNEL \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x14, AcquireChannelRequest)

/**
 * Get all the platforms found in current frequency.
 * Paramters:	IsLockedRequest struct
 */
#define IOCTL_ITE_DEMOD_ISLOCKED \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x15, IsLockedRequest)

/**
 * Get the statistic values of demodulator, it includes Pre-Viterbi BER,
 * Post-Viterbi BER, Abort Count, Signal Presented Flag, Signal Locked Flag,
 * Signal Quality, Signal Strength, Delta-T for DVB-H time slicing.
 * Paramters:	GetStatisticRequest struct
 */
#define IOCTL_ITE_DEMOD_GETSTATISTIC \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x18, GetStatisticRequest)

/**
 * Get the statistic values of demodulator, it includes Pre-Viterbi BER,
 * Post-Viterbi BER, Abort Count, Signal Presented Flag, Signal Locked Flag,
 * Signal Quality, Signal Strength, Delta-T for DVB-H time slicing.
 * Paramters:	GetChannelStatisticRequest struct
 */
#define IOCTL_ITE_DEMOD_GETCHANNELSTATISTIC \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x19, GetChannelStatisticRequest)

/**
 * Paramters: 	ControlPowerSavingRequest struct
 */
#define IOCTL_ITE_DEMOD_CONTROLPOWERSAVING \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x1E, ControlPowerSavingRequest)

/**
 * Modulator Set Modulation.
 * Paramters: 	TxSetModuleRequest struct
 */
#define IOCTL_ITE_MOD_SETMODULE \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x21, TxSetModuleRequest)

/**
 * Modulator Acquire Channel.
 * Paramters: 	TxAcquireChannelRequest struct
 */	
#define IOCTL_ITE_MOD_ACQUIRECHANNEL \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x22, TxAcquireChannelRequest)

/**
 * Modulator Null Packet Enable.
 * Paramters: 	TxModeRequest struct
 */	
#define IOCTL_ITE_MOD_ENABLETXMODE \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x23, TxModeRequest)
	
/**
 * Read a sequence of bytes from the contiguous registers in demodulator.
 * Paramters:   ReadRegistersRequest struct
 */
#define IOCTL_ITE_MOD_READREGISTERS \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x24, TxReadRegistersRequest)

/**
 * Write a sequence of bytes to the contiguous registers in demodulator.
 * Paramters:   TxWriteRegistersRequest struct
 */
#define IOCTL_ITE_MOD_WRITEREGISTERS \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x27, TxWriteRegistersRequest)
	
/**
 * Modulator Device Type Setting.
 * Paramters:   TxSetDeviceTypeRequest struct
 */	
#define IOCTL_ITE_MOD_SETDEVICETYPE \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x28, TxSetDeviceTypeRequest)

/**
 * Modulator Device Type Getting.
 * Paramters:   TxGetDeviceTypeRequest struct
 */	
#define IOCTL_ITE_MOD_GETDEVICETYPE \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x29, TxGetDeviceTypeRequest)

/**
 * Modulator Set Gain Range.
 * Paramters:   TxSetGainRequest struct
 */	
#define IOCTL_ITE_MOD_ADJUSTOUTPUTGAIN \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x2B, TxSetGainRequest)

/**
 * Modulator Get Gain Range.
 * Paramters:   TxGetGainRangeRequest struct
 */	
#define IOCTL_ITE_MOD_GETGAINRANGE \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x2C, TxGetGainRangeRequest)

/**
 * Modulator Get Output Gain Range.
 * Paramters:   TxGetOutputGainRangeRequest struct
 */	
#define IOCTL_ITE_MOD_GETOUTPUTGAIN \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x2D, TxGetOutputGainRequest)

/**
 * Paramters: 	TxControlPowerSavingRequest struct
 */
#define IOCTL_ITE_MOD_CONTROLPOWERSAVING \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x2F, TxControlPowerSavingRequest)

/**
 * Write a sequence of bytes to the contiguous cells in the EEPROM.
 * Paramters:   WriteEepromValuesRequest struct
 */
#define IOCTL_ITE_MOD_WRITEEEPROMVALUES \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x31, TxWriteEepromValuesRequest)

/**
 * Read a sequence of bytes from the contiguous cells in the EEPROM.
 * Paramters:   ReadEepromValuesRequest struct
 */
#define IOCTL_ITE_MOD_READEEPROMVALUES \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x32, TxReadEepromValuesRequest)

/**
 * Get Chip Type IT9507/IT9503 in modulator.
 * Paramters:   TxGetChipTypeRequest struct
 */
#define IOCTL_ITE_MOD_GETCHIPTYPE \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x3B, TxGetChipTypeRequest)

/**
 * Get Chip Type IT9507/IT9503 in modulator.
 * Paramters:   TxSetSpectralInversion struct
 */
#define IOCTL_ITE_MOD_SETSPECTRALINVERSION \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_STANDARD + 0x3C, TxSetSpectralInversionRequest)

/***************************************************************************/
/*                           DVBT                                 */
/***************************************************************************/
/**
 * Reset PID from PID filter.
 * Paramters:	ResetPidRequest struct
 */
#define IOCTL_ITE_DEMOD_RESETPID \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_DVBT + 0x02, ResetPidRequest)

/**
 * Enable PID filter.
 * Paramters:	ControlPidFilterRequest struct
 */
#define IOCTL_ITE_DEMOD_CONTROLPIDFILTER \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_DVBT + 0x03, ControlPidFilterRequest)

/**
 * Add PID to PID filter.
 * Paramters:	AddPidAtRequest struct
 */
#define IOCTL_ITE_DEMOD_ADDPIDAT \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_DVBT + 0x04, AddPidAtRequest)

/**
 * Add PID to PID filter.
 * Paramters:	AddPidAtRequest struct
 */
#define IOCTL_ITE_MOD_ADDPIDAT \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_DVBT + 0x08, TxAddPidAtRequest)

/**
 * Reset PID from PID filter.
 * Paramters:	ResetPidRequest struct
 */
#define IOCTL_ITE_MOD_RESETPID \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_DVBT + 0x10, TxResetPidRequest)

/**
 * Enable PID filter.
 * Paramters:	TxControlPidFilterRequest struct
 */
#define IOCTL_ITE_MOD_CONTROLPIDFILTER \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_DVBT + 0x11, TxControlPidFilterRequest)

/**
 * Enable Set IQTable From File.
 * Paramters:	TxSetIQTableRequest struct
 */
#define IOCTL_ITE_MOD_SETIQTABLE \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_DVBT + 0x12, TxSetIQTableRequest)

/**
 * Enable Set DC Calibration Value From File.
 * Paramters:	TxSetDCCalibrationValueRequest struct
 */
#define IOCTL_ITE_MOD_SETDCCALIBRATIONVALUE \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_DVBT + 0x13, TxSetDCCalibrationValueRequest)


/***************************************************************************/
/*                            OTHER                               */
/***************************************************************************/
/**
 * Get driver information.
 * Paramters: 	DemodDriverInfo struct
 */
#define IOCTL_ITE_DEMOD_GETDRIVERINFO \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_OTHER + 0x00, DemodDriverInfo)

/**
 * Start capture data stream
 * Paramters: StartCaptureRequest struct
 */
#define IOCTL_ITE_DEMOD_STARTCAPTURE \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_OTHER + 0x01, StartCaptureRequest)

/**
 * Stop capture data stream
 * Paramters: StopCaptureRequest struct
 */
#define IOCTL_ITE_DEMOD_STOPCAPTURE \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_OTHER + 0x02, StopCaptureRequest)

/**
 * Start Transfer data stream
 * Paramters: StartTransferRequest struct
 */
#define IOCTL_ITE_MOD_STARTTRANSFER \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_OTHER + 0x07, TxStartTransferRequest)

/**
 * Stop capture data stream
 * Paramters: StopTransferRequest struct
 */	
#define IOCTL_ITE_MOD_STOPTRANSFER \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_OTHER + 0x08, TxStopTransferRequest)

/**
 * Modulator: Get Driver information.
 * Paramters: TxModDriverInfo struct
 */		
#define IOCTL_ITE_MOD_GETDRIVERINFO \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_OTHER + 0x09, TxModDriverInfo)
	
/**
 * Modulator: Set Start Transfer data Streaming.
 * Paramters: StopTransferRequest struct
 */	
#define IOCTL_ITE_MOD_STARTTRANSFER_CMD \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_OTHER + 0x0A, TxStartTransferRequest)
	
/**
 * Modulator: Set Stop Transfer data Streaming.
 * Paramters: TxStopTransferRequest struct
 */		
#define IOCTL_ITE_MOD_STOPTRANSFER_CMD \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_OTHER + 0x0B, TxStopTransferRequest)
	
/**
 * Modulator: Set Command.
 * Paramters: TxCmdRequest struct
 */		
#define IOCTL_ITE_MOD_WRITE_CMD \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_OTHER + 0x0C, TxCmdRequest)	
	
/**
 * Modulator: Get TPS.
 * Paramters: TxGetTPSRequest struct
 */	
#define IOCTL_ITE_MOD_GETTPS \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_OTHER + 0x0D, TxGetTPSRequest)	

/**
 * Modulator: Set TPS.
 * Paramters: TxSetTPSRequest struct
 */		
#define IOCTL_ITE_MOD_SETTPS \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_OTHER + 0x0E, TxSetTPSRequest)		

/**
 * Modulator: Send PSI Table to Hardware.
 * Paramters: TxSetTPSRequest struct
 */	
#define IOCTL_ITE_MOD_SENDHWPSITABLE \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_OTHER + 0x0F, TxSendHwPSITableRequest)		

/**
 * Modulator: Access PSI Table to firmware.
 * Paramters: TxSetTPSRequest struct
 */		
#define IOCTL_ITE_MOD_ACCESSFWPSITABLE \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_OTHER + 0x10, TxAccessFwPSITableRequest)		

/**
 * Modulator: Access PSI Table to firmware.
 * Paramters: TxSetTPSRequest struct
 */	
#define IOCTL_ITE_MOD_SETFWPSITABLETIMER \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_OTHER + 0x11, TxSetFwPSITableTimerRequest)		

/**
 * Modulator: Write Low Bit Rate Date.
 * Paramters: TxSetLowBitRateTransferRequest struct
 */	
#define IOCTL_ITE_MOD_WRITE_LOWBITRATEDATA \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_OTHER + 0x12, TxSetLowBitRateTransferRequest)	

/**
 * Modulator: Set PCR Mode.
 * Paramters: TxSetPcrModeRequest struct
 */	
#define IOCTL_ITE_MOD_SETPCRMODE \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_OTHER + 0x13, TxSetPcrModeRequest)	

/**
 * Modulator: Set DC Table.
 * Paramters: TxSetPcrModeRequest struct
 */	
#define IOCTL_ITE_MOD_SETDCTABLE \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_OTHER + 0x14, TxSetDCTableRequest)	

/**
 * Enable Get Frequency Index Value From API.
 * Paramters:	GetFrequencyIndexRequest struct
 */
#define IOCTL_ITE_MOD_GETFREQUENCYINDEX \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_OTHER + 0x15, TxGetFrequencyIndexRequest)	

	
/***************************************************************************/
/*                            ISDB-T                             */
/***************************************************************************/
/**
 * Set ISDB-T Channel Modulation.
 * Paramters:	TXSetISDBTChannelModulationRequest struct
 */
#define IOCTL_ITE_MOD_SETISDBTCHANNELMODULATION \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_ISDBT + 0x00, TXSetISDBTChannelModulationRequest)

/**
 * Set TMCC Information.
 * Paramters:	TXSetTMCCInfoRequest struct
 */
#define IOCTL_ITE_MOD_SETTMCCINFO \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_ISDBT + 0x01, TXSetTMCCInfoRequest)
	
/**
 * Get TMCC Information.
 * Paramters:	TXGetTMCCInfoRequest struct
 */
#define IOCTL_ITE_MOD_GETTMCCINFO \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_ISDBT + 0x02, TXGetTMCCInfoRequest)

/**
 * Get TS Input Bit Rate.
 * Paramters:	TXGetTSinputBitRate struct
 */
#define IOCTL_ITE_MOD_GETTSINPUTBITRATE \
	_IOR(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_ISDBT + 0x03, TXGetTSinputBitRateRequest)
	
/**
 * Get Add Pid To ISDBT Pid Filter.
 * Paramters:	TXGetTSinputBitRate struct
 */
#define IOCTL_ITE_MOD_ADDPIDTOISDBTPIDFILTER \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_ISDBT + 0x04, TXAddPidToISDBTPidFilterRequest)
		
/**
 * Get DTV Mode.
 * Paramters:	TxGetDTVModeRequest struct
 */
#define IOCTL_ITE_MOD_GETDTVMODE \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_ISDBT + 0x05, TxGetDTVModeRequest)

/***************************************************************************/
/*                             SECURITY                          */
/***************************************************************************/
/**
 * Enable TPS Encryption.
 * Paramters:	TxEnableTpsEncryptionRequest struct
 */
#define IOCTL_ITE_MOD_ENABLETPSENCRYPTION \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_SECURITY + 0x01, TxEnableTpsEncryptionRequest)

/**
 * Disable TPS Encryption.
 * Paramters:	TxDisableTpsEncryptionRequest struct
 */
#define IOCTL_ITE_MOD_DISABLETPSENCRYPTION \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_SECURITY + 0x02, TxDisableTpsEncryptionRequest)

/**
 * Set TPS Decryption.
 * Paramters:	TxSetDecryptRequest struct
 */
#define IOCTL_ITE_DEMOD_SETDECRYPT \
	_IOW(AFA_IOC_MAGIC, IOCTRL_ITE_GROUP_SECURITY + 0x03, TxSetDecryptRequest)

Dword DemodIOCTLFun(
    void *       		handle,
    Dword       		IOCTLCode,
    unsigned long     pIOBuffer);

#endif
