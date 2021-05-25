/**
 * Copyright (c) 2016 ITE Corporation. All rights reserved. 
 *
 * Module Name:
 *   IT950x Core 
 *
 * Abstract:
 *   The core of IT950x serial driver.
 */
 
#include "it950x-core.h"
#include "iocontrol.h"

//static DEFINE_MUTEX(it950x_mutex);
static DEFINE_MUTEX(it950x_urb_kill);
static DEFINE_MUTEX(it950x_rb_mutex);
static Byte tx_usb_handles[TX_MAX_USB_HANDLE_NUM] = {0};
static Byte rx_usb_handles[RX_MAX_USB_HANDLE_NUM] = {0};


#define to_afa_dev(d) container_of(d, struct it950x_dev, kref)

static struct usb_driver  it950x_driver;

struct usb_device_id it950x_usb_id_table[] = {
		{ USB_DEVICE(0x048D,0x9507) },
		{ 0},		/* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, it950x_usb_id_table);

/* AirHD */
Dword Tx_RMRingBuffer(struct it950x_urb_context *context, Dword dwDataFrameSize)
{
	struct it950x_dev *dev = context->dev;
	unsigned long flags = 0;

#if RB_DEBUG
	deb_data("Enter %s", __func__);
	deb_data("Tx_RMRingBuffer: dev->TxCurrBuffPointAddr %lu", dev->TxCurrBuffPointAddr);
#endif
	spin_lock_irqsave(&dev->TxRBKeyLock, flags);
		//dev->TxCurrBuffPointAddr = (dev->TxCurrBuffPointAddr + dwDataFrameSize) % (dev->dwTxWriteTolBufferSize);// add by EE 0110
		//dev->dwTxRemaingBufferSize += dwDataFrameSize;// add by EE 0110
		dev->TxCurrBuffPointAddr = (dev->TxCurrBuffPointAddr + URB_BUFSIZE_TX) % (dev->dwTxWriteTolBufferSize);// add by EE 0110
		dev->dwTxRemaingBufferSize += URB_BUFSIZE_TX;// add by EE 0110
		dev->tx_urb_counter++;
	spin_unlock_irqrestore(&dev->TxRBKeyLock, flags);

	dev->tx_urbstatus[context->index] = 0;

#ifdef TSDUCK_WRITE
	wake_up_interruptible(&dev->tx_urb_wait);
#endif
	return 0;
}

/* AirHD for low bitrate */
Dword Tx_RMRingBuffer_low_brate(struct it950x_urb_context *context, Dword dwDataFrameSize)
{
	struct it950x_dev *dev = context->dev;
	unsigned long flags = 0;	

#if RB_DEBUG
	deb_data("Enter %s", __func__);
	deb_data("Tx_RMRingBuffer_low_brate: dev->TxCurrBuffPointAddr %lu", dev->TxCurrBuffPointAddr_low_brate);
#endif
	spin_lock_irqsave(&dev->TxRBKeyLock_low_brate, flags);
		dev->TxCurrBuffPointAddr_low_brate = (dev->TxCurrBuffPointAddr_low_brate + URB_BUFSIZE_TX_LOW_BRATE) % (dev->dwTxWriteTolBufferSize_low_brate);
		dev->dwTxRemaingBufferSize_low_brate += URB_BUFSIZE_TX_LOW_BRATE;
		dev->tx_urb_counter_low_brate++;
	spin_unlock_irqrestore(&dev->TxRBKeyLock_low_brate, flags);

	dev->tx_urbstatus_low_brate[context->index] = 0;
	return 0;
}

/* AirHD for cmd */
#if RETURN_CHANNEL
Dword Tx_RMRingBuffer_cmd(struct it950x_urb_context *context, Dword dwDataFrameSize)
{
	struct it950x_dev *dev = context->dev;
	//deb_data("enter %s", __func__);
	//deb_data("RMRingBuffer: (*dev->pWriteCurrBuffPointAddr) %d", (*dev->pWriteCurrBuffPointAddr));
	unsigned long flags;
	
	spin_lock_irqsave(&dev->TxRBKeyLock_cmd, flags);
	//spin_lock_irqsave(&Write_RingBuffer_Lock, flags);
	*dev->pTxCurrBuffPointAddr_cmd =
			(*dev->pTxCurrBuffPointAddr_cmd + dwDataFrameSize) % dev->dwTxWriteTolBufferSize_cmd;

	dev->dwTxRemaingBufferSize_cmd += dwDataFrameSize;
	dev->tx_urbstatus_cmd[context->index] = 0;
	spin_unlock_irqrestore(&dev->TxRBKeyLock_cmd, flags);

	return 0;
}
#endif
void Rx_FillRingBuffer(
	struct it950x_dev *dev,
	Dword data_size)
{ 
	unsigned long flags = 0;
	
	/*
	 * dwRxCurrBuffPointAddr : Return_urb addr (allow reading addr)
	 * dwRxTolBufferSize : total ringbuffer size
	 */
	spin_lock_irqsave(&dev->RxRBKeylock, flags);
		dev->RxCurrBuffPointAddr = (dev->RxCurrBuffPointAddr + data_size) % (dev->dwRxTolBufferSize);
		dev->dwRxReadRemaingBufferSize += data_size;
	spin_unlock_irqrestore(&dev->RxRBKeylock, flags);
	
	atomic_add(1, &dev->rx_urb_counter);
	
	if(atomic_read(&dev->rx_urb_counter) == URB_COUNT_RX){
#if RB_DEBUG		
		deb_data("\t %s warning: RingBuffer already full\n", __func__);
#endif
	}
}

static void
rx_free_urbs(struct it950x_dev *dev)
{
	int i;
	//deb_data("Enter %s Function\n",__FUNCTION__);
	
	for (i = 0; i < URB_COUNT_RX; i++) {
		usb_free_urb(dev->rx_urbs[i]);
		dev->rx_urbs[i] = NULL;
	}
	
	deb_data("%s() end\n", __func__);
}
static void
tx_free_urbs(struct it950x_dev *dev)
{
	int i;
	//deb_data("Enter %s Function\n",__FUNCTION__);
	
	for (i = 0; i < URB_COUNT_TX; i++) {
		usb_free_urb(dev->tx_urbs[i]);
		dev->tx_urbs[i] = NULL;
	}
	for(i = 0; i < URB_COUNT_TX_LOW_BRATE; i++){
		usb_free_urb(dev->tx_urbs_low_brate[i]);
		dev->tx_urbs_low_brate[i] = NULL;
	}			
#if RETURN_CHANNEL	
	for(i = 0; i < URB_COUNT_TX_CMD; i++){
		usb_free_urb(dev->tx_urbs_cmd[i]);
		dev->tx_urbs_cmd[i] = NULL;
	}
#endif
	deb_data("%s() end\n", __func__);
}
/*
static void
rx_kill_busy_urbs(struct it950x_dev *dev)
{
	int i;
	//deb_data("Enter %s Function\n",__FUNCTION__);
	
	for (i = 0; i < URB_COUNT_RX; i++) {
		if(dev->urbstatus_rx[i] == 1){
			usb_kill_urb(dev->urbs_rx[i]);
			deb_data("kill rx urb index %d\n", i);
		}
	}
	
	deb_data("%s() end\n", __func__);
}
static void
tx_kill_busy_urbs(struct it950x_dev *dev)
{
	int i;
	//deb_data("Enter %s Function\n",__FUNCTION__);
	
	for (i = 0; i < URB_COUNT_TX; i++) {
		if(dev->tx_urbstatus[i] == 1){
			usb_kill_urb(dev->tx_urbs[i]);
			deb_data("kill tx urb index %d\n", i);
		}
	}
	
	deb_data("%s() end\n", __func__);
}
*/
static int rx_stop_urb_transfer(struct it950x_dev *dev)
{
#if 0
	int i;
#endif
	if (!dev->rx_urb_streaming) {
		deb_data("%s: iso xfer already stop!\n", __func__);
		return 0;
	}

	dev->rx_urb_streaming = 0;
	
 	/*DM368 usb bus error when using kill urb */
#if 0
	msleep(1000);  						  /* waiting urb complete. */
	//mutex_lock(&it950x_urb_kill);	     /* if urb doesn't call back, kill it. */
	for (i = 0; i < URB_COUNT_RX; i++) {
		if(dev->rx_urbstatus[i] == 1){
			usb_kill_urb(dev->rx_urbs[i]);
			deb_data("kill rx urb index %d\n", i);
		}
	}
	//mutex_unlock(&it950x_urb_kill);		
#endif

	deb_data("%s() end\n", __func__);

	return 0;
}

static int tx_stop_urb_transfer(struct it950x_dev *dev)
{
	//deb_data("%s()\n", __func__);

	if (!dev->tx_urb_streaming && !dev->tx_urb_streaming_low_brate) {
		deb_data("%s: iso xfer already stop!\n", __func__);
		return 0;
	}

	dev->tx_urb_streaming = 0;
	dev->tx_urb_streaming_low_brate = 0;
	
#ifdef TSDUCK_WRITE
        /* make sure that processes waiting for buffer are resumed */
	wake_up_interruptible(&dev->tx_urb_wait);
#endif

	//memset(dev->pTxRingBuffer, 0, get_order(dev->dwTxWriteTolBufferSize));
	if(!dev->pTxRingBuffer) {
//		deb_data("Clean pTxRingBuffer error\n", dev->pTxRingBuffer);
		return -ENOMEM;
	}

	
	/*DM368 usb bus error when using kill urb */
#if 0
	for (i = 0; i < URB_COUNT_TX; i++) {
		if(dev->tx_urbstatus[i] == 1) {
			dev->tx_urbstatus[i] = 0;
			usb_kill_urb(dev->tx_urbs[i]);
		}
		usb_free_urb(dev->tx_urbs[i]);
		dev->tx_urbs[i] = NULL;
	}

	mutex_lock(&it950x_urb_kill);
	tx_kill_busy_urbs(dev);
	mutex_unlock(&it950x_urb_kill);
#endif

	deb_data("%s() end\n", __func__);

	return 0;
}
#if RETURN_CHANNEL
static int tx_stop_urb_transfer_cmd(struct it950x_dev *dev)
{
	deb_data("%s()\n", __func__);

	if (!dev->tx_urb_streaming_cmd) {
		deb_data("%s: iso xfer already stop!\n", __func__);
		return 0;
	}
	
	dev->tx_urb_streaming_cmd = 0;
	
	/*DM368 usb bus error when using kill urb */
#if 0
	for (i = 0; i < URB_COUNT_TX; i++) {
		if(dev->tx_urbstatus[i] == 1) {
			dev->tx_urbstatus[i] = 0;
			usb_kill_urb(dev->tx_urbs[i]);
		}
		usb_free_urb(dev->tx_urbs[i]);
		dev->tx_urbs[i] = NULL;
	}

	mutex_lock(&it950x_urb_kill);
	tx_kill_busy_urbs(dev);
	mutex_unlock(&it950x_urb_kill);
#endif

	deb_data("%s() end\n", __func__);

	return 0;
}
#endif
DWORD fabs_self(DWORD a, DWORD b)
{
	DWORD c = 0;
	
	c = a - b;
	if(c >= 0) return c;
	else return c * -1;
}

/**
 * Tx_RingBuffer - Eagle the core of write Ring-Buffer.
 * Description:
 * Successful submissions return 0(Error_NO_ERROR) and submised buffer length.
 * Otherwise this routine returns a negative error number.
 */
DWORD Tx_RingBuffer(
	struct it950x_dev *dev,
    Byte* pBuffer,
    Dword* pBufferLength)
{
    Dword dwBuffLen = 0;
    Dword dwCpBuffLen = *pBufferLength;
    Dword dwCurrBuffAddr = dev->TxCurrBuffPointAddr;
    Dword dwWriteBuffAddr = dev->TxWriteBuffPointAddr;
	Dword remainingSize = dev->dwTxRemaingBufferSize;
	int ret = -ENOMEM;

#if RB_DEBUG
	deb_data("RemaingBufferSize: {%lu}\n", dev->dwTxRemaingBufferSize);	
	deb_data("Tx_RingBuffer-CPLen %lu, dwCurrBuffAddr %lu, dwWriteBuffAddr %lu\n", dwCpBuffLen, dwCurrBuffAddr, dwWriteBuffAddr);
#endif
	/* RingBuffer full */
	if ((remainingSize) == 0) {
		*pBufferLength = 0;
#if RB_DEBUG		
		deb_data("dwTxRemaingBufferSize = 0\n");
#endif
		return Error_BUFFER_INSUFFICIENT;
	}

    if ((remainingSize) < dwCpBuffLen) {
		*pBufferLength = 0;
#if RB_DEBUG
		deb_data("dwTxRemaingBufferSize < dwCpBuffLen\n");
#endif
		return Error_BUFFER_INSUFFICIENT;
	}

    if (*pBufferLength == 0) {
		if (dev->tx_urb_streaming == 1) {
			//add by EE 0110
			/* allow submit urb */
			if (dwWriteBuffAddr % URB_BUFSIZE_TX == 0){
//				deb_data("\n\n\n%s(add by EE): \t\t@@@@@@@@@@@@@@ Good no submit\n\n\n", __func__);
				return Error_NO_ERROR;
			}
			dwBuffLen = dwWriteBuffAddr - (dev->tx_urb_index * URB_BUFSIZE_TX);
			if (dwBuffLen < 0) {
				deb_data("\n\n\n%s(add by EE): dwBuffLen < 0, dwBuffLen = %lu\n\n\n", __func__, dwBuffLen);
				return Error_BUFFER_INSUFFICIENT;
			}
			dev->tx_urbs[dev->tx_urb_index]->transfer_buffer_length = dwBuffLen;
			ret = usb_submit_urb(dev->tx_urbs[dev->tx_urb_index], GFP_ATOMIC);
			if (ret != 0) {
				tx_stop_urb_transfer(dev);
				deb_data("%s(add by EE): failed urb submission, err = %d\n", __func__, ret);
				return ret;
			}

			dev->TxWriteBuffPointAddr = (dev->TxWriteBuffPointAddr + (URB_BUFSIZE_TX - dwBuffLen)) % (dev->dwTxWriteTolBufferSize);
			dev->dwTxRemaingBufferSize -= (URB_BUFSIZE_TX - dwBuffLen);

			dev->tx_urbstatus[dev->tx_urb_index] = 1;
			dev->tx_urb_index = (dev->tx_urb_index + 1) % URB_COUNT_TX;
			dev->tx_urb_counter--;
		}		
		
        return Error_NO_ERROR;
    }

	/* memory must enough because checking at first in this function */
	if (dwWriteBuffAddr >= dwCurrBuffAddr) {
		/* To_kernel_urb not run a cycle or both run a cycle */
		dwBuffLen = dev->dwTxWriteTolBufferSize - dwWriteBuffAddr;
		if (dwBuffLen >= dwCpBuffLen) {
//			memcpy(dev->pTxRingBuffer + dwWriteBuffAddr, pBuffer, dwCpBuffLen);
			if (copy_from_user(dev->pTxRingBuffer + dwWriteBuffAddr, pBuffer, dwCpBuffLen))
				return -EFAULT;
		}
		else {
//			memcpy(dev->pTxRingBuffer + dwWriteBuffAddr, pBuffer, dwBuffLen);
//			memcpy(dev->pTxRingBuffer, pBuffer + dwBuffLen, dwCpBuffLen - dwBuffLen);
			if (copy_from_user(dev->pTxRingBuffer + dwWriteBuffAddr, pBuffer, dwBuffLen))
				return -EFAULT;
			if (copy_from_user(dev->pTxRingBuffer, pBuffer + dwBuffLen, dwCpBuffLen - dwBuffLen))
				return -EFAULT;
		}
	}
	else {
		/* To_kernel_urb run a cycle and Return_urb not */
//		memcpy(dev->pTxRingBuffer + dwWriteBuffAddr, pBuffer, dwCpBuffLen);
		if (copy_from_user(dev->pTxRingBuffer + dwWriteBuffAddr, pBuffer, dwCpBuffLen))
			return -EFAULT;
	}

	dev->TxWriteBuffPointAddr = (dev->TxWriteBuffPointAddr + dwCpBuffLen) % (dev->dwTxWriteTolBufferSize);
	dev->dwTxRemaingBufferSize -= dwCpBuffLen;
	dwWriteBuffAddr = dev->TxWriteBuffPointAddr;

	if (dev->tx_urb_streaming == 1) {
		/* allow submit urb */
		while (((dwWriteBuffAddr - (dev->tx_urb_index * URB_BUFSIZE_TX)) >= URB_BUFSIZE_TX ||
			(dwWriteBuffAddr < (dev->tx_urb_index * URB_BUFSIZE_TX))) && dev->tx_urb_counter > 0) {
			/* while urb full and not submit */
			dev->tx_urbs[dev->tx_urb_index]->transfer_buffer_length = URB_BUFSIZE_TX; //add by EE 0110
			ret = usb_submit_urb(dev->tx_urbs[dev->tx_urb_index], GFP_ATOMIC);
			if (ret != 0) {
				tx_stop_urb_transfer(dev);
#if RB_DEBUG				
				deb_data("%s: failed urb submission, err = %d\n", __func__, ret);
#endif
				return ret;
			}
			
			dev->tx_urbstatus[dev->tx_urb_index] = 1;
			dev->tx_urb_index = (dev->tx_urb_index + 1) % URB_COUNT_TX;
			dev->tx_urb_counter--;
		}
	}
   
    return Error_NO_ERROR;
}

/* AirHD low bitrate */
DWORD Tx_RingBuffer_low_brate(
	struct it950x_dev *dev,
    Byte*  pBuffer,
    Dword* pBufferLength)
{
    Dword dwBuffLen = 0;
    Dword dwCpBuffLen = *pBufferLength;
    Dword dwCurrBuffAddr = dev->TxCurrBuffPointAddr_low_brate;
    Dword dwWriteBuffAddr = dev->TxWriteBuffPointAddr_low_brate;
    Dword remainingSize = dev->dwTxRemaingBufferSize_low_brate;
    int ret = -ENOMEM;

#if RB_DEBUG
	deb_data("RemaingBufferSize_low_brate: {%lu}\n", dev->dwTxRemaingBufferSize_low_brate);
	deb_data("Tx_RingBuffer_low_brate-CPLen %lu, dwCurrBuffAddr_low_brate %lu, dwWriteBuffAddr_low_brate %lu\n", 
		dwCpBuffLen, dwCurrBuffAddr, dwWriteBuffAddr);
#endif
	/* RingBuffer full */
	if ((remainingSize) == 0) {
		*pBufferLength = 0;
#if RB_DEBUG
		deb_data("dwTxRemaingBufferSize_low_brate = 0\n");
#endif
		return Error_BUFFER_INSUFFICIENT;
	}

    if ((remainingSize) < dwCpBuffLen) {
		*pBufferLength = 0;
#if RB_DEBUG
		deb_data("dwTxRemaingBufferSize_low_brate < dwCpBuffLen\n");
#endif
		return Error_BUFFER_INSUFFICIENT;
	}

	if (*pBufferLength == 0) {
		if (dev->tx_urb_streaming_low_brate == 1) {
			dwBuffLen = dwWriteBuffAddr - (dev->tx_urb_index_low_brate * URB_BUFSIZE_TX_LOW_BRATE);
			if (dwBuffLen > 0) {
				dev->tx_urbs_low_brate[dev->tx_urb_index_low_brate]->transfer_buffer_length = dwBuffLen;
				ret = usb_submit_urb(dev->tx_urbs_low_brate[dev->tx_urb_index_low_brate], GFP_ATOMIC);
				if (ret != 0) {
					tx_stop_urb_transfer(dev);
					deb_data("%s: failed urb submission immediately _low_brate, err = %d\n", __func__, ret);
					return ret;
				}

				dev->tx_urbstatus_low_brate[dev->tx_urb_index_low_brate] = 1;
				dev->tx_urb_index_low_brate = (dev->tx_urb_index_low_brate + 1) % URB_COUNT_TX_LOW_BRATE;
				dev->tx_urb_counter_low_brate--;

				dev->TxWriteBuffPointAddr_low_brate = dev->tx_urb_index_low_brate * URB_BUFSIZE_TX_LOW_BRATE;
				dev->dwTxRemaingBufferSize_low_brate -= (URB_BUFSIZE_TX_LOW_BRATE - dwBuffLen);
			}
		}

		return Error_NO_ERROR;
	}

#if 0
    if (*pBufferLength == 0) {
		if (dev->tx_urb_streaming_low_brate == 1) {
			//add by EE 0110
			/* allow submit urb */
			if (dwWriteBuffAddr % URB_BUFSIZE_TX_LOW_BRATE == 0) {
				//deb_data("\n\n\n%s(add by EE): \t\t@@@@@@@@@@@@@@ Good no submit\n\n\n", __func__);
				return Error_NO_ERROR;
			}

			dwBuffLen = dwWriteBuffAddr - (dev->tx_urb_index_low_brate * URB_BUFSIZE_TX_LOW_BRATE);
			if (dwBuffLen < 0) {
				deb_data("\n\n\n%s(add by EE): dwBuffLen < 0, dwBuffLen = %lu\n\n\n", __func__, dwBuffLen);
				return Error_BUFFER_INSUFFICIENT;
			}

			dev->tx_urbs_low_brate[dev->tx_urb_index_low_brate]->transfer_buffer_length = dwBuffLen;
			ret = usb_submit_urb(dev->tx_urbs_low_brate[dev->tx_urb_index_low_brate], GFP_ATOMIC);
			if (ret != 0) {
				tx_stop_urb_transfer(dev);
				deb_data("%s(add by EE): failed urb submission, err = %d\n", __func__, ret);
				return ret;
			}

			dev->TxWriteBuffPointAddr_low_brate = (dev->TxWriteBuffPointAddr_low_brate + (URB_BUFSIZE_TX_LOW_BRATE - dwBuffLen)) % (dev->dwTxWriteTolBufferSize_low_brate);
			dev->dwTxRemaingBufferSize_low_brate -= (URB_BUFSIZE_TX_LOW_BRATE - dwBuffLen);

			dev->tx_urbstatus_low_brate[dev->tx_urb_index_low_brate] = 1;
			dev->tx_urb_index_low_brate = (dev->tx_urb_index_low_brate + 1) % URB_COUNT_TX_LOW_BRATE;
			dev->tx_urb_counter_low_brate--;
		}		
		
        return Error_NO_ERROR;
    }
#endif

	/* memory must enough because checking at first in this function */
	if (dwWriteBuffAddr >= dwCurrBuffAddr) {
		/* To_kernel_urb not run a cycle or both run a cycle */
		dwBuffLen = dev->dwTxWriteTolBufferSize_low_brate - dwWriteBuffAddr;
		if (dwBuffLen >= dwCpBuffLen) {
//			memcpy(dev->pTxRingBuffer_low_brate + dwWriteBuffAddr, pBuffer, dwCpBuffLen);
			if (copy_from_user(dev->pTxRingBuffer_low_brate + dwWriteBuffAddr, pBuffer, dwCpBuffLen))
				return -EFAULT;
		}
		else {
//			memcpy(dev->pTxRingBuffer_low_brate + dwWriteBuffAddr, pBuffer, dwBuffLen);
			if (copy_from_user(dev->pTxRingBuffer_low_brate + dwWriteBuffAddr, pBuffer, dwBuffLen))
				return -EFAULT;
//			memcpy(dev->pTxRingBuffer_low_brate, pBuffer + dwBuffLen, dwCpBuffLen - dwBuffLen);
			if (copy_from_user(dev->pTxRingBuffer_low_brate, pBuffer + dwBuffLen, dwCpBuffLen - dwBuffLen))
				return -EFAULT;
		}
	}
	else {
//		memcpy(dev->pTxRingBuffer_low_brate + dwWriteBuffAddr, pBuffer, dwCpBuffLen);
		if (copy_from_user(dev->pTxRingBuffer_low_brate + dwWriteBuffAddr, pBuffer, dwCpBuffLen))
			return -EFAULT;
	}

	dev->TxWriteBuffPointAddr_low_brate = (dev->TxWriteBuffPointAddr_low_brate + dwCpBuffLen) % (dev->dwTxWriteTolBufferSize_low_brate);
	dev->dwTxRemaingBufferSize_low_brate -= dwCpBuffLen;
	dwWriteBuffAddr = dev->TxWriteBuffPointAddr_low_brate;

	if (dev->tx_urb_streaming_low_brate == 1) {
		/* allow submit urb */
		while (((dwWriteBuffAddr - (dev->tx_urb_index_low_brate * URB_BUFSIZE_TX_LOW_BRATE)) >= URB_BUFSIZE_TX_LOW_BRATE ||
			(dwWriteBuffAddr < (dev->tx_urb_index_low_brate * URB_BUFSIZE_TX_LOW_BRATE))) && dev->tx_urb_counter_low_brate > 0) {
			/* while urb full and not submit */
			dev->tx_urbs_low_brate[dev->tx_urb_index_low_brate]->transfer_buffer_length = URB_BUFSIZE_TX_LOW_BRATE; //add by EE 0110
			ret = usb_submit_urb(dev->tx_urbs_low_brate[dev->tx_urb_index_low_brate], GFP_ATOMIC);
			if (ret != 0) {
				tx_stop_urb_transfer(dev);
#if RB_DEBUG
				deb_data("%s: failed urb submission _low_brate, err = %d\n", __func__, ret);
#endif
				return ret;
			}
			
			dev->tx_urbstatus_low_brate[dev->tx_urb_index_low_brate] = 1;
			dev->tx_urb_index_low_brate = (dev->tx_urb_index_low_brate + 1) % URB_COUNT_TX_LOW_BRATE;
			dev->tx_urb_counter_low_brate--;
		}
	}
   
    return Error_NO_ERROR;
}

/* AirHD_CMD */
#if RETURN_CHANNEL
DWORD Tx_RingBuffer_cmd(
	struct it950x_dev *dev,
    Byte* pBuffer,
    Dword* pBufferLength)
{
	unsigned long flags = 0;
	DWORD dwBuffLen = 0;
	DWORD dwCpBuffLen = *pBufferLength;
	DWORD dwCurrBuffAddr = (*dev->pTxCurrBuffPointAddr_cmd);
	DWORD dwWriteBuffAddr = (*dev->pTxWriteBuffPointAddr_cmd);
	Dword error = Error_NO_ERROR;
	bool RingBufferOverWrite = false;
	
	
	spin_lock_irqsave(&dev->TxRBKeyLock_cmd, flags);
	
	if (*pBufferLength == 0)
	{
		error = Error_BUFFER_INSUFFICIENT;
		goto exit;
	}

	if((dev->dwTxRemaingBufferSize_cmd == 0) || (dev->dwTxRemaingBufferSize_cmd < dwCpBuffLen))
	{
		*pBufferLength = 0;
		error = Error_NO_ERROR;
		goto exit;
	}

	if(dwWriteBuffAddr >= dwCurrBuffAddr)
	{
		dwBuffLen = dev->dwTxWriteTolBufferSize_cmd - dwWriteBuffAddr;
		if(dwBuffLen < dwCpBuffLen)
		{
//			memcpy(dev->pWriteFrameBuffer_cmd+dwWriteBuffAddr, pBuffer, dwBuffLen);
			if (copy_from_user(dev->pWriteFrameBuffer_cmd+dwWriteBuffAddr, pBuffer, dwBuffLen))
				return -EFAULT;
//			memcpy(dev->pWriteFrameBuffer_cmd, pBuffer+dwBuffLen, dwCpBuffLen-dwBuffLen);
			if (copy_from_user(dev->pWriteFrameBuffer_cmd, pBuffer+dwBuffLen, dwCpBuffLen-dwBuffLen))
				return -EFAULT;
			RingBufferOverWrite = true;
		}
		else {
//			memcpy(dev->pWriteFrameBuffer_cmd+dwWriteBuffAddr, pBuffer, dwCpBuffLen);
			if (copy_from_user(dev->pWriteFrameBuffer_cmd+dwWriteBuffAddr, pBuffer, dwCpBuffLen))
				return -EFAULT;
		}
	}
	else {
//		memcpy(dev->pWriteFrameBuffer_cmd+dwWriteBuffAddr, pBuffer, dwCpBuffLen);
		if (copy_from_user(dev->pWriteFrameBuffer_cmd+dwWriteBuffAddr, pBuffer, dwCpBuffLen)
				return -EFAULT;
	}


	*dev->pTxWriteBuffPointAddr_cmd = (*dev->pTxWriteBuffPointAddr_cmd+dwCpBuffLen) % dev->dwTxWriteTolBufferSize_cmd;
	dev->dwTxRemaingBufferSize_cmd -= *pBufferLength;


	if((*dev->pTxWriteBuffPointAddr_cmd) < (dev->tx_urb_index_cmd*URB_BUFSIZE_TX_CMD))
	{
		while(1)
		{
			if(!dev->tx_urbstatus_cmd[dev->tx_urb_index_cmd])
			{
				error = usb_submit_urb(dev->tx_urbs_cmd[dev->tx_urb_index_cmd], GFP_ATOMIC);
				if (error != 0) {
					tx_stop_urb_transfer_cmd(dev);
					deb_data("%s: failed urb submission, err = %lu\n", __func__, error);
					goto exit;
				}//else deb_data("usb_submit_urb cmd ok \n");

				dev->tx_urbstatus_cmd[dev->tx_urb_index_cmd] = 1;
			}

			dev->tx_urb_index_cmd++;
			if(dev->tx_urb_index_cmd == URB_COUNT_TX_CMD)
			{
				dev->tx_urb_index_cmd = 0;
				break;
			}
		}
	}

	while(((*dev->pTxWriteBuffPointAddr_cmd - (dev->tx_urb_index_cmd*URB_BUFSIZE_TX_CMD)) >= URB_BUFSIZE_TX_CMD)
			|| (RingBufferOverWrite == true))
	{
		if(!dev->tx_urbstatus_cmd[dev->tx_urb_index_cmd])
		{
			error = usb_submit_urb(dev->tx_urbs_cmd[dev->tx_urb_index_cmd], GFP_ATOMIC);
			if (error != 0) {
				tx_stop_urb_transfer_cmd(dev);
				deb_data("%s: failed urb submission, err = %lu\n", __func__, error);
				goto exit;
			}//else deb_data("usb_submit_urb cmd ok \n");

			dev->tx_urbstatus_cmd[dev->tx_urb_index_cmd] = 1;
		}

		dev->tx_urb_index_cmd++;
		if(dev->tx_urb_index_cmd == URB_COUNT_TX_CMD)
		{
			dev->tx_urb_index_cmd = 0;
			RingBufferOverWrite = false;
		}
	}

exit:

	spin_unlock_irqrestore(&dev->TxRBKeyLock_cmd, flags);

    return error;

}
#endif
DWORD Rx_RingBuffer(
	struct it950x_dev *dev,
	Byte *pBuffer,
	Dword *pBufferLength)
{
	Dword dwCpBuffLen = *pBufferLength;
	Dword dwCurrBuffAddr = dev->RxCurrBuffPointAddr;
	Dword dwReadBuffAddr = dev->RxReadBuffPointAddr;
	Dword dwBuffLen = 0;
	unsigned long flags = 0;
	int ret = -ENOMEM;
	
	//deb_data("- Enter %s Function -\n", __func__);
//	deb_data("RemaingBufferSize: {%lu}\n", dev->dwRxReadRemaingBufferSize);		
//	deb_data("Tx_RingBuffer-CPLen %lu, dwCurrBuffAddr %lu, dwWriteBuffAddr %lu\n", dwCpBuffLen, dwCurrBuffAddr, dwWriteBuffAddr);
	
	if((dev->dwRxReadRemaingBufferSize) == 0){
		*pBufferLength = 0;
//		deb_data("\t Warning: ReadRemaingBufferSize = 0\n");
		return Error_NO_ERROR;
	}

    if(*pBufferLength == 0){
		*pBufferLength = 0;
        return Error_BUFFER_INSUFFICIENT;
    }
    
    if((dev->dwRxReadRemaingBufferSize) < dwCpBuffLen){
		*pBufferLength = 0;
		//deb_data("\t Warning: ReadRemaingBufferSize(%lu) < Request(%lu)\n", chip->dwReadRemaingBufferSize, dwCpBuffLen);
		return Error_NO_ERROR;
	}
	
	/*
	 * dwReadBuffAddr : already read addr
	 * dwCurrBuffAddr : Return_urb addr (allow reading addr)
	 * dwTolBufferSize : total ringbuffer size
	 */
	
	/* memory must enough because already checking */
	
	//RingBuffer
	if(dwReadBuffAddr >= dwCurrBuffAddr){
		//Return_urb run a cycle and already_read not
		dwBuffLen = dev->dwRxTolBufferSize - dwReadBuffAddr; //remaining memory (to buffer end), not contain buffer beginning to Return_urb
		if(dwBuffLen >= dwCpBuffLen){
			//end remaining memory is enough
//			memcpy(pBuffer, dev->pRxRingBuffer + dwReadBuffAddr, dwCpBuffLen);
			if (copy_to_user(pBuffer, dev->pRxRingBuffer + dwReadBuffAddr, dwCpBuffLen))
				return -EFAULT;
		}
		else{
			//use all end memory, run a cycle and need use beginning memory
//			memcpy(pBuffer, dev->pRxRingBuffer + dwReadBuffAddr, dwBuffLen); //using end memory
			if (copy_to_user(pBuffer, dev->pRxRingBuffer + dwReadBuffAddr, dwBuffLen)) //using end memory
				return -EFAULT;
//			memcpy(pBuffer + dwBuffLen, dev->pRxRingBuffer, dwCpBuffLen - dwBuffLen); //using begining memory
			if (copy_to_user(pBuffer + dwBuffLen, dev->pRxRingBuffer, dwCpBuffLen - dwBuffLen)) //using begining memory
				return -EFAULT;
		}
	}
	else{
		//Return_urb not run a cycle or both run a cycle
//		memcpy(pBuffer, dev->pRxRingBuffer + dwReadBuffAddr, dwCpBuffLen);
		if (copy_to_user(pBuffer, dev->pRxRingBuffer + dwReadBuffAddr, dwCpBuffLen))
			return -EFAULT;
	}
	
	spin_lock_irqsave(&dev->RxRBKeylock, flags);
		dev->RxReadBuffPointAddr = (dev->RxReadBuffPointAddr + dwCpBuffLen) % (dev->dwRxTolBufferSize);
		dev->dwRxReadRemaingBufferSize -= dwCpBuffLen;
	spin_unlock_irqrestore(&dev->RxRBKeylock, flags);
	
	//Submit urb
	if(dev->rx_urb_streaming == 1 && dev->rx_first_urb_reset == 1){
		//allow submit urb && first urb already resubmit
		while((dev->RxReadBuffPointAddr - (dev->rx_urb_index * URB_BUFSIZE_RX)) >= URB_BUFSIZE_RX && atomic_read(&dev->rx_urb_counter) > 0){
			//while urb empty and not submit
			ret = usb_submit_urb(dev->rx_urbs[dev->rx_urb_index], GFP_ATOMIC);
			if (ret != 0) {
				rx_stop_urb_transfer(dev);
				deb_data("\t Error: urb[%d] resubmit fail, err = %d\n",dev->rx_urb_index , ret);
				return ret;
			}
			
			dev->rx_urbstatus[dev->rx_urb_index] = 1;
			dev->rx_urb_index = (dev->rx_urb_index + 1) % URB_COUNT_RX;
			atomic_sub(1, &dev->rx_urb_counter);
		}
	}
	
	return Error_NO_ERROR;
}

/******************************************************************/

static void tx_urb_completion(struct urb *purb, struct pt_regs *ppt_regs)
{
	struct it950x_urb_context *context = purb->context;
	int ptype = usb_pipetype(purb->pipe);
	
#if RB_DEBUG	
	deb_data("Enter %s", __func__);

	deb_data("urb id: %d. '%s' urb completed. status: %d, length: %d/%d, pack_num: %d, errors: %d\n",
		context->index,
		ptype == PIPE_ISOCHRONOUS ? "isoc" : "bulk",
		purb->status,purb->actual_length,purb->transfer_buffer_length,
		purb->number_of_packets,purb->error_count);
#endif		
	//context->dev->tx_urbstatus[context->index] = 0;
	switch (purb->status) {
		case 0:         /* success */
		case -ETIMEDOUT:    /* NAK */
			break;
		case -ECONNRESET:   /* kill */
		case -ENOENT:
			//context->dev->urb_use_count++;
			deb_data("TX ENOENT-urb completition error %d.\n", purb->status);
		case -ESHUTDOWN:
			return;
		default:        /* error */
			deb_data("TX urb completition error %d.\n", purb->status);
			break;
	}

	if (!context->dev) return;
	
	//if (context->dev->tx_urb_streaming == 0) return;

	if (ptype != PIPE_BULK) {
		deb_data("TX %s() Unsupported URB type %d\n", __func__, ptype);
		return;
	}
	
	//ptr = (u8 *)purb->transfer_buffer;
	/* Feed the transport payload into the kernel demux */
	//dvb_dmx_swfilter_packets(&dev->dvb.demux,
	//	purb->transfer_buffer, purb->actual_length / 188);
	//if (purb->actual_length > 0)
	
	Tx_RMRingBuffer(context, URB_BUFSIZE_TX);
	
	return;
}

static void tx_urb_completion_low_brate(struct urb *purb, struct pt_regs *ppt_regs)
{
	struct it950x_urb_context *context = purb->context;
	int ptype = usb_pipetype(purb->pipe);
	
#if RB_DEBUG	
	deb_data("Enter %s", __func__);

	deb_data("_low_brate urb id: %d. '%s' urb completed. status: %d, length: %d/%d, pack_num: %d, errors: %d\n",
		context->index,
		ptype == PIPE_ISOCHRONOUS ? "isoc" : "bulk",
		purb->status,purb->actual_length,purb->transfer_buffer_length,
		purb->number_of_packets,purb->error_count);
#endif		
	//context->dev->tx_urbstatus[context->index] = 0;
	switch (purb->status) {
		case 0:         /* success */
		case -ETIMEDOUT:    /* NAK */
			break;
		case -ECONNRESET:   /* kill */
		case -ENOENT:
			//context->dev->urb_use_count++;
			deb_data("TX_low_brate ENOENT-urb completition error %d.\n", purb->status);
		case -ESHUTDOWN:
			return;
		default:        /* error */
			deb_data("TX_low_brate urb completition error %d.\n", purb->status);
			break;
	}

	if (!context->dev) return;
	
	//if (context->dev->tx_urb_streaming_low_brate == 0) return;

	if (ptype != PIPE_BULK) {
		deb_data("TX_low_brate %s() Unsupported URB type %d\n", __func__, ptype);
		return;
	}
	
	//ptr = (u8 *)purb->transfer_buffer;
	/* Feed the transport payload into the kernel demux */
	//dvb_dmx_swfilter_packets(&dev->dvb.demux,
	//	purb->transfer_buffer, purb->actual_length / 188);
	//if (purb->actual_length > 0)
	
	Tx_RMRingBuffer_low_brate(context, URB_BUFSIZE_TX_LOW_BRATE);
	
	return;
}
#if RETURN_CHANNEL
static void tx_urb_completion_cmd(struct urb *purb, struct pt_regs *ppt_regs)
{
	struct it950x_urb_context *context = purb->context;
	int ptype = usb_pipetype(purb->pipe);
	//deb_data("enter %s", __func__);

	//deb_data("'%s' urb completed. status: %d, length: %d/%d, pack_num: %d, errors: %d\n",
	//	ptype == PIPE_ISOCHRONOUS ? "isoc" : "bulk",
	//	purb->status,purb->actual_length,purb->transfer_buffer_length,
	//	purb->number_of_packets,purb->error_count);
	
	//context->dev->tx_urbstatus_cmd[context->index] = 0;
	switch (purb->status) {
		case 0:         /* success */
		case -ETIMEDOUT:    /* NAK */
			break;
		case -ECONNRESET:   /* kill */
		case -ENOENT:
			//context->dev->urb_use_count_cmd++;
			deb_data("TX ENOENT-urb completition error %d.\n", purb->status);
		case -ESHUTDOWN:
			return;
		default:        /* error */
			deb_data("TX urb completition error %d.\n", purb->status);
			break;
	}

	if (!context->dev) return;
	
	if (context->dev->tx_urb_streaming_cmd == 0) return;

	if (ptype != PIPE_BULK) {
		deb_data("TX %s() Unsupported URB type %d\n", __func__, ptype);
		return;
	}
	
	//ptr = (u8 *)purb->transfer_buffer;
	/* Feed the transport payload into the kernel demux */
	//dvb_dmx_swfilter_packets(&dev->dvb.demux,
	//	purb->transfer_buffer, purb->actual_length / 188);
	//if (purb->actual_length > 0)
	
	Tx_RMRingBuffer_cmd(context, URB_BUFSIZE_TX_CMD);
	
	return;
}
#endif
static void rx_urb_completion(struct urb *purb, struct pt_regs *ppt_regs)
{
	struct it950x_urb_context *context = purb->context;
	struct it950x_dev *dev = context->dev;	
	int ptype = usb_pipetype(purb->pipe);
	int ret = -ENOMEM;

	//deb_data("%s", __func__);
	//deb_data("'%s' urb completed. status: %d, length: %d/%d, pack_num: %d, errors: %d\n",
	//	ptype == PIPE_ISOCHRONOUS ? "isoc" : "bulk",
	//	purb->status,purb->actual_length,purb->transfer_buffer_length,
	//	purb->number_of_packets,purb->error_count);
	//deb_data("urb_complete(%d)\n", context->index);

	if(purb->actual_length != URB_BUFSIZE_RX)	
		deb_data("\t Warning: Rx_urb[%d].length = %d\n", context->index, purb->actual_length);
			
#if PACKECT_SHIFT	 // Pack Shift issue. 
	 else {
		if(dev->pRxRingBuffer[context->index * URB_BUFSIZE_RX] != 0x47){
			deb_data("\t Warning: Rx_urb[%d] not 0x47\n", context->index);
			dev->buffer_shift = 1;
			wake_up_interruptible(&(dev->reset_wait));
		}		
	}
#endif
		
	context->dev->rx_urbstatus[context->index] = 0;
	switch (purb->status) {
		case 0:              /* Success */
			break;
		case -ETIMEDOUT:     /* NAK */
			deb_data("RX ETIMEDOUT -urb completition error %d.\n", purb->status);
			break;
		case -ECONNRESET:    /* unlink */
			deb_data("RX ECONNRESET -urb completition error %d.\n", purb->status);
			return;
		case -ENOENT:        /* kill */
			deb_data("RX ENOENT -urb completition error %d.\n", purb->status);
			return;
		case -ESHUTDOWN:
			deb_data("RX ESHUTDOWN -urb completition error %d.\n", purb->status);
			return;
		default:             /* error */
			deb_data("RX urb completition error %d.\n", purb->status);
			break;
	}

	if (!dev){
		deb_data("dev is NULL\n");
		return;
	}

	if (ptype != PIPE_BULK) {
		deb_data("RX %s() Unsupported URB type %d\n", __func__, ptype);
		return;
	}

	/* For solve first urb not correct problem */
	if(dev->rx_urb_streaming){
		if(!dev->rx_first_urb_reset){
			ret = usb_submit_urb(dev->rx_urbs[0], GFP_ATOMIC);
			if(ret != 0){
				rx_stop_urb_transfer(dev);
				deb_data("\t%s error: urb[0] resubmit fail, err = %d\n", __func__ , ret);
				return ;
			}
			else{
				//deb_data("\t urb[0] resubmit success\n");
			}
			
			dev->rx_urbstatus[0] = 1;
			dev->rx_urb_index = (dev->rx_urb_index + 1) % URB_COUNT_RX;
			dev->rx_first_urb_reset = 1;
		}
		else{
			Rx_FillRingBuffer(dev, URB_BUFSIZE_RX);
		}
	}
	else{
		if(context->index != 0)
			Rx_FillRingBuffer(dev, URB_BUFSIZE_RX);
	}
}

/* AirHD */
static int tx_start_urb_transfer(struct it950x_dev *dev)
{

	//int i; 
	int ret = -ENOMEM;

	deb_data("%s()\n", __func__);
	if (dev->tx_urb_streaming || dev->tx_urb_streaming_low_brate) {
		deb_data("%s: iso xfer already running!\n", __func__);
		return 0;
	}

	/* Write ring buffer */
	dev->TxCurrBuffPointAddr = 0;
	dev->TxWriteBuffPointAddr = 0;
	dev->dwTxRemaingBufferSize = dev->dwTxWriteTolBufferSize;
	dev->tx_urb_index = 0;
	dev->tx_urb_streaming = 1;
	dev->tx_urb_counter = URB_COUNT_TX;
	
	/* Write ring buffer low brate */
	dev->TxCurrBuffPointAddr_low_brate = 0;
	dev->TxWriteBuffPointAddr_low_brate = 0;
	dev->dwTxRemaingBufferSize_low_brate = dev->dwTxWriteTolBufferSize_low_brate;
	dev->tx_urb_index_low_brate = 0;
	dev->tx_urb_streaming_low_brate = 1;
	dev->tx_urb_counter_low_brate = URB_COUNT_TX_LOW_BRATE;
	
	ret = 0;

	deb_data("%s() end\n", __func__);

	return ret;
}

/* AirHD_CMD */
#if RETURN_CHANNEL
static int tx_start_urb_transfer_cmd(struct it950x_dev *dev)
{
	int ret = -ENOMEM;

	deb_data("%s()\n", __func__);

	if (dev->tx_urb_streaming_cmd) {
		deb_data("%s: iso xfer already running!\n", __func__);
		return 0;
	}
	
	*(dev->pTxCurrBuffPointAddr_cmd) = 0;
	*(dev->pTxWriteBuffPointAddr_cmd) = 0;
	dev->dwTxRemaingBufferSize_cmd = dev->dwTxWriteTolBufferSize_cmd;
	dev->tx_urb_index_cmd = 0;
	dev->tx_urb_streaming_cmd = 1;
	ret = 0;

	deb_data("%s() end\n", __func__);
	return ret;
}
#endif
static void rx_start_urb_transfer(struct it950x_dev *dev)
{
	int urb_index = 0, ret = -ENOMEM;

	deb_data("- Enter %s Function -\n", __func__);

	dev->rx_urb_index = 0;
	dev->RxCurrBuffPointAddr = URB_BUFSIZE_RX;	//first time skip urb[0]
	dev->RxReadBuffPointAddr = URB_BUFSIZE_RX;	//first time skip urb[0]
	dev->rx_first_urb_reset = 0;
	dev->dwRxReadRemaingBufferSize = 0;
	atomic_set(&dev->rx_urb_counter, URB_COUNT_RX);
	
	for(urb_index = 0; urb_index < URB_COUNT_RX; urb_index++){
		ret = usb_submit_urb(dev->rx_urbs[urb_index], GFP_ATOMIC);
		if(ret != 0) {
			rx_stop_urb_transfer(dev);
			deb_data("\t AError: urb[%d] submit fail, err = %d\n", urb_index, ret);
			return ;
		}
		else{
			//deb_data("\t urb[%d] submit success\n", urb_index);
		}
		
		dev->rx_urbstatus[urb_index] = 1;
		dev->rx_urb_index = (dev->rx_urb_index + 1) % URB_COUNT_RX;
		atomic_sub(1, &dev->rx_urb_counter);
	}
	
	dev->rx_urb_streaming = 1;
	return ;
}

static int it950x_usb_open(struct inode *inode, struct file *file)
{
	struct it950x_dev *dev;
	struct usb_interface *interface;
	int subminor, mainsubminor;
	int retval = 0;
	int error/*order,*/;

	deb_data("it950x_usb_rx_open function\n");
	mainsubminor = iminor(inode);
	subminor = iminor(inode);
	interface = usb_find_interface(&it950x_driver, subminor);

try:
	while (!interface) {
		subminor++;
		interface = usb_find_interface(&it950x_driver, subminor);
		if (subminor >= mainsubminor + USB_it913x_MINOR_RANGE)
			break;
	}	
	
	if (!interface) {
		deb_data("%s - error, can't find device for minor %d",
		     __FUNCTION__, subminor);
		retval = -ENODEV;
		goto exit;
	}
	
	dev = usb_get_intfdata(interface);
	if (!dev) {
		deb_data("usb_get_intfdata fail!\n");
		retval = -ENODEV;
		goto exit;
	}
	
	if (subminor != dev->tx_chip_minor) {
		interface = NULL;
		goto try;
	}	
	deb_data("open RX subminor: %d\n", subminor);		
	atomic_add(1, &dev->g_AP_use_rx);		
	
	if(	atomic_read(&dev->g_AP_use_rx) == 1) {	// Prevent multi-open the same minor of APs.
#if DEVICE_POWER_CTRL
		if(atomic_read(&dev->rx_pw_on) == 0) {
			if(atomic_read(&dev->tx_pw_on) == 0) {
				error = DL_ApPwCtrl(&dev, 0, 1);
				error = DL_ApPwCtrl(&dev->DC, 1, 1);
				atomic_set(&dev->tx_pw_on, 1);	
				atomic_set(&dev->rx_pw_on, 1);				
			} else {		
				error = DL_ApPwCtrl(&dev->DC, 1, 1);
				atomic_set(&dev->rx_pw_on, 1);	
			}
		}	
#endif			
	}	
	/* increment our usage count for the device */
	//kref_get(&dev->kref);
	
	/* save our object in the file's private structure */
	dev->file = file;
	file->private_data = dev;

exit:
	return retval;
}

static int it950x_usb_tx_open(struct inode *inode, struct file *file)
{
	struct it950x_dev *dev;
	struct usb_interface *interface;
	int mainsubminor, subminor;
	int retval = 0;
	int error;

	deb_data("it950x_usb_tx_open function\n");
	
	mainsubminor = iminor(inode);	
	subminor = iminor(inode);
	interface = usb_find_interface(&it950x_driver, subminor);

try:
	while (!interface) {
		subminor++;
		interface = usb_find_interface(&it950x_driver, subminor);
		if (subminor >= mainsubminor + USB_it950x_MINOR_RANGE)
			break;
	}		
	
	if (!interface) {
		deb_data("%s - error, can't find device for minor %d",
		     __FUNCTION__, subminor);
		retval = -ENODEV;
		goto exit;
	}
	dev = usb_get_intfdata(interface);
	
	if (!dev) {
		deb_data("usb_get_intfdata fail!\n");
		retval = -ENODEV;
		goto exit;
	}
	
	if (subminor != dev->tx_chip_minor) {
		interface = NULL;
		goto try;
	}	
	deb_data("open TX subminor: %d\n", subminor);			
	atomic_add(1, &dev->g_AP_use_tx);

/*
 *  Allocate buffer just for first user, 
 *  in order to prevent multi-open the same minor of APs.
 */
	if( atomic_read(&dev->g_AP_use_tx) == 1) {     
#if DEVICE_POWER_CTRL
		if(atomic_read(&dev->tx_pw_on) == 0) {
			error = DL_ApPwCtrl(&dev->DC, 0, 1);
			atomic_set(&dev->tx_pw_on, 1);	
		}
#endif	
	}
	
	/* save our object in the file's private structure */
	dev->tx_file = file;
	file->private_data = dev;
	
	/* init API IQ table for default */
//	dev->DC.modulator.calibrationInfo.ptrIQtableEx = NULL;
//	IT9507_setIQtable ((Modulator*) &(dev->DC.modulator), dev->DC.modulator.calibrationInfo);	

exit:
	return retval;
}

static int it950x_usb_release(struct inode *inode, struct file *file)
{
	struct it950x_dev *dev;
	int error, i;
	//int order;

	dev = (struct it950x_dev *)file->private_data;
	if (dev == NULL) {
		deb_data("dev is NULL\n");
		return -ENODEV;
	}
	
	if(atomic_read(&dev->g_AP_use_rx) == 1) {   // Prevent multi-open the same minor of APs.
		rx_stop_urb_transfer(dev);

		dev = (struct it950x_dev *)file->private_data;
		if (dev == NULL)
			return -ENODEV;

		/* decrement the count on our device */
		//kref_put(&dev->kref, afa_delete);

		/* if urb doesn't call back, kill it. */
		for (i = 0; i < URB_COUNT_RX; i++) {   
			if(dev->rx_urbstatus[i] == 1){
				usb_kill_urb(dev->rx_urbs[i]);
			}
		}	

#if DEVICE_POWER_CTRL
		if(atomic_read(&dev->rx_pw_on) == 1) {
			if(atomic_read(&dev->g_AP_use_tx) == 0) {    // normal.
				error = DL_ApPwCtrl(&dev->DC, 1, 0);
				error = DL_ApPwCtrl(&dev->DC, 0, 0);
				atomic_set(&dev->rx_pw_on, 0);	
				atomic_set(&dev->tx_pw_on, 0);				
			} else {		                                 // if tx in used. just off rx.
				error = DL_ApPwCtrl(&dev->DC, 1, 0);
				atomic_set(&dev->rx_pw_on, 0);	
			}
		}	
#endif		
	}
	atomic_sub(1, &dev->g_AP_use_rx);
	return 0;
}

static int it950x_usb_tx_release(struct inode *inode, struct file *file)
{
	struct it950x_dev *dev;
	int error;

	//deb_data("it950x_usb_release function\n");	
	dev = (struct it950x_dev *)file->private_data;
	if (dev == NULL) {
		deb_data("dev is NULL\n");
		return -ENODEV;
	}	

	if(	atomic_read(&dev->g_AP_use_tx) == 1) {

		tx_stop_urb_transfer(dev);
#if RETURN_CHANNEL
		tx_stop_urb_transfer_cmd(dev);
#endif		
		/* decrement the count on our device */
		//kref_put(&dev->kref, afa_delete);
		
#if DEVICE_POWER_CTRL
		if(atomic_read(&dev->tx_pw_on) == 1) {
			if(atomic_read(&dev->g_AP_use_rx) == 0) {   // RX not used, suspend tx.
				error = DL_ApPwCtrl(&dev->DC, 0, 0);
				atomic_set(&dev->tx_pw_on, 0);	
			} else {
				deb_data("RX lock TX_PowerSaving\n");
			}
		}
#endif		

	}
	atomic_sub(1, &dev->g_AP_use_tx);

	return 0;
}

int SetLowBitRateTransfer(struct it950x_dev *dev, void *parg)
{
	DWORD dwError = Error_NO_ERROR;
	Dword *pdwBufferLength = 0;
//	Byte *lbr_buffer = NULL;
	unsigned long flags = 0;
	
	PTxSetLowBitRateTransferRequest pRequest = (PTxSetLowBitRateTransferRequest) parg;
	pdwBufferLength = (Dword*) pRequest->pdwBufferLength;		

	//lbr_buffer = (Byte*) kmalloc(sizeof(Byte)*(unsigned long) (*pdwBufferLength), GFP_KERNEL);
	//ret = copy_from_user(lbr_buffer, (Byte*) pRequest->pBufferAddr,(unsigned long) (*pdwBufferLength));
	//dwError = Tx_RingBuffer_low_brate(dev, lbr_buffer, pdwBufferLength);
	spin_lock_irqsave(&dev->TxRBKeyLock_low_brate, flags);
	dwError = Tx_RingBuffer_low_brate(dev, (Byte*) pRequest->pBufferAddr, pdwBufferLength);
	spin_unlock_irqrestore(&dev->TxRBKeyLock_low_brate, flags);
//	deb_data("lbr_buffer[%x][%x]\n", lbr_buffer[0], lbr_buffer[1]);
//	deb_data("pdwBufferLength: %lu\n", pdwBufferLength);

//	if(lbr_buffer) kfree((void*)lbr_buffer);
	
	pRequest->error = dwError;
	return 0;
}

#if PACKECT_SHIFT
int Thread_buffer_reset(void *DEV)
{
	struct it951x_dev *dev = (struct it951x_dev *) DEV;
	
	deb_data("- Create %s Thread -\n", __func__);
	
	init_waitqueue_head(&dev->reset_wait);
	
	while(!kthread_should_stop()){
		wait_event_interruptible(dev->reset_wait, (dev->buffer_shift == 1) || (dev->disconnect == 1));
		
		if(dev->disconnect == 1)
			break;
		
		DL_ResetBuffer((void*) &dev->DC);
		dev->buffer_shift = 0;
	}	

	deb_data("\t %s thread kill\n", __func__);
	
	return 0;
}
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,35)
static int it950x_usb_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd,  unsigned long parg)
{
	struct it950x_dev *dev;
  
	dev = (struct it950x_dev *)file->private_data;
	if (dev == NULL) {
		deb_data("dev is NULL\n");
		return -ENODEV;
	}

	/*
	* extract the type and number bitfields, and don't decode
	* wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	*/

	//if (_IOC_TYPE(cmd) != AFA_IOC_MAGIC) return -ENOTTY;
	//if (_IOC_NR(cmd) > AFA_IOC_MAXNR) return -ENOTTY;

	switch (cmd)
	{	
		case IOCTL_ITE_DEMOD_STARTCAPTURE:
			rx_start_urb_transfer(dev);
			return 0;		
		
		case IOCTL_ITE_DEMOD_STOPCAPTURE:
			rx_stop_urb_transfer(dev);
			return 0;
	}
	return DL_DemodIOCTLFun((void *)&dev->DC.demodulator, (DWORD)cmd, parg);
}
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
long it950x_usb_unlocked_ioctl(
	struct file *file,
	unsigned int cmd,
	unsigned long parg)
{
	struct it950x_dev *dev;
  	dev = (struct it950x_dev *)file->private_data;
	if (dev == NULL) {
		deb_data("dev is NULL\n");
		return -ENODEV;
	}

	/*
	* extract the type and number bitfields, and don't decode
	* wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	*/

	//if (_IOC_TYPE(cmd) != AFA_IOC_MAGIC) return -ENOTTY;
	//if (_IOC_NR(cmd) > AFA_IOC_MAXNR) return -ENOTTY;

	switch (cmd)
	{	
		case IOCTL_ITE_DEMOD_STARTCAPTURE:
			rx_start_urb_transfer(dev);
			return 0;		
		
		case IOCTL_ITE_DEMOD_STOPCAPTURE:
			rx_stop_urb_transfer(dev);
			return 0;
	}
	return DL_DemodIOCTLFun((void *)&dev->DC.demodulator, (DWORD)cmd, parg);
}
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,35)
static int it950x_usb_tx_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd,  unsigned long parg)
{
	struct it950x_dev	*dev;
    PDEVICE_CONTEXT		handle;
	SystemConfig		syscfg;
	PTxCmdRequest		pTxCmdRequest;
	TxSetDeviceTypeRequest pTxSetDeviceTypeRequest;
	TxGetDeviceTypeRequest pTxGetDeviceTypeRequest;
	PTxSetIQTableRequest 	pSetIQTableRequest;
	Byte *pIQtableUser, *pIQtableKernel;
	Word tableGroups, tableVersion;
	//Byte temp;
	int	 i, j;

	//deb_data("it951x_usb_tx_ioctl function\n");
	
	dev = (struct it950x_dev *)file->private_data;
	if (dev == NULL) {
		deb_data("dev is NULL\n");
		return -ENODEV;
	}

	handle = &dev->DC;
   /*
	* extract the type and number bitfields, and don't decode
	* wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	*/
	//if (_IOC_TYPE(cmd) != AFA_IOC_MAGIC) return -ENOTTY;
	//if (_IOC_NR(cmd) > AFA_IOC_MAXNR) return -ENOTTY;
    
	switch (cmd)
	{
		case IOCTL_ITE_MOD_STARTTRANSFER:
			tx_start_urb_transfer(dev);
			return 0;
					
		case IOCTL_ITE_MOD_STOPTRANSFER:
			tx_stop_urb_transfer(dev);
			return 0;
			
		case IOCTL_ITE_MOD_STARTTRANSFER_CMD:
#if RETURN_CHANNEL
			tx_start_urb_transfer_cmd(dev);
#endif			
			return 0;
			
		case IOCTL_ITE_MOD_STOPTRANSFER_CMD:
#if RETURN_CHANNEL		
			tx_stop_urb_transfer_cmd(dev);
#endif			
			return 0;
			
		case IOCTL_ITE_MOD_WRITE_CMD:
#if RETURN_CHANNEL
			pTxCmdRequest = (PTxCmdRequest) parg;
			Tx_RingBuffer_cmd(dev, (Byte*) pTxCmdRequest->cmdAddr, (Dword*) &pTxCmdRequest->len);
#endif
			return 0;
			
		case IOCTL_ITE_MOD_WRITE_LOWBITRATEDATA:
			SetLowBitRateTransfer(dev, (void*)parg);
			return 0;
			
		case IOCTL_ITE_MOD_GETDEVICETYPE:
		
			if (copy_from_user((void *)&pTxGetDeviceTypeRequest, (void *)parg, sizeof(TxGetDeviceTypeRequest)))
				return -EFAULT;

			if(handle->isUsingDefaultDeviceType == true){	// is Using Default devicetype. return default type and return error message.
				pTxGetDeviceTypeRequest.DeviceType = handle->deviceType;
				pTxGetDeviceTypeRequest.error = ModulatorError_INVALID_DEV_TYPE;
			} else {											// no using default. return current device type.
				pTxGetDeviceTypeRequest.DeviceType = handle->deviceType;			
				pTxGetDeviceTypeRequest.error = Error_NO_ERROR;			
			}
																																				                       			
			if (copy_to_user((void *)parg, (void *)&pTxGetDeviceTypeRequest, sizeof(TxGetDeviceTypeRequest)))
				return -EFAULT;
/*
			pTxGetDeviceTypeRequest = (PTxGetDeviceTypeRequest) parg;
			
			if(handle->isUsingDefaultDeviceType == true){	// is Using Default devicetype. return default type and return error message.
				pTxGetDeviceTypeRequest->DeviceType = handle->deviceType;
				pTxGetDeviceTypeRequest->error = ModulatorError_INVALID_DEV_TYPE;			
			} else {		// no using default. return current device type.
				pTxGetDeviceTypeRequest->DeviceType = handle->deviceType;			
				pTxGetDeviceTypeRequest->error = Error_NO_ERROR;			
			}*/
			return 0;
			
		case IOCTL_ITE_MOD_SETDEVICETYPE:
			if (copy_from_user((void *)&pTxSetDeviceTypeRequest, (void *)parg, sizeof(TxGetDeviceTypeRequest)))
				return -EFAULT;

			pTxSetDeviceTypeRequest.error = EagleUser_getSystemConfig((Modulator*) &(handle->modulator), pTxSetDeviceTypeRequest.DeviceType, &syscfg);
			if(pTxSetDeviceTypeRequest.error != Error_NO_ERROR)
				deb_data("EagleUser_getSystemConfig fail\n\t- DeviceType id: %d\n", pTxSetDeviceTypeRequest.DeviceType);
			else 
				deb_data("EagleUser_getSystemConfig ok\n\t- DeviceType id: %d\n", pTxSetDeviceTypeRequest.DeviceType);
			
			handle->isUsingDefaultDeviceType = false;	// Set for no default device type.
			handle->deviceType = pTxSetDeviceTypeRequest.DeviceType;
			handle->modulator.systemConfig = syscfg;

			pTxSetDeviceTypeRequest.error = EagleUser_setSystemConfig((Modulator*) &(handle->modulator), syscfg);
			if(pTxSetDeviceTypeRequest.error != Error_NO_ERROR)
				deb_data("EagleUser_setSystemConfig fail\n");	
			else 
				deb_data("EagleUser_setSystemConfig ok\n");			

			/* re-init GPIO setting for new device type */
			pTxSetDeviceTypeRequest.error = EagleUser_Initialization((Modulator*) &(handle->modulator));
			if(pTxSetDeviceTypeRequest.error != Error_NO_ERROR)		
				deb_data("EagleUser_Initialization fail\n");	
			else 														
				deb_data("EagleUser_Initialization ok\n");				

			if (copy_to_user((void *)parg, (void *)&pTxSetDeviceTypeRequest, sizeof(TxGetDeviceTypeRequest)))
				return -EFAULT;
			return 0;				

		case IOCTL_ITE_MOD_SETIQTABLE:
			pSetIQTableRequest = (PTxSetIQTableRequest) parg;
			pIQtableUser = (Byte*) pSetIQTableRequest->pIQtableAddr;

			if(pIQtableUser != NULL) {
				tableGroups = (Word) (pIQtableUser[14]<<8 | pIQtableUser[15]);
				tableVersion = (Dword) (pIQtableUser[10]<< 16 | pIQtableUser[11]<< 8 | pIQtableUser[12]);				
				if( (tableGroups*8+16) != pSetIQTableRequest->IQtableSize) {
						deb_data("[SETIQTable] Error: File size and table groups is different.\n");
						pSetIQTableRequest->error = ModulatorError_INVALID_DATA_LENGTH;				
						return 0;
				}

				pIQtableKernel = (Byte*) __get_free_pages(GFP_KERNEL, get_order(tableGroups*8+16));
				if (copy_from_user(pIQtableKernel, pIQtableUser, (tableGroups*8+16)))
					return -EFAULT;
				if(dev->DC.calibrationInfo.tableGroups != tableGroups) {		// Reallocate table.
					deb_data("[SETIQTable] Reallocate IQ Table.\n");
					if(dev->DC.calibrationInfo.ptrIQtableEx) {
						free_pages((unsigned long) dev->DC.calibrationInfo.ptrIQtableEx, get_order(dev->DC.calibrationInfo.tableGroups));
						dev->DC.calibrationInfo.ptrIQtableEx = NULL;
					}
					dev->DC.calibrationInfo.tableGroups = tableGroups;
					dev->DC.calibrationInfo.ptrIQtableEx = (IQtable*)__get_free_pages(GFP_KERNEL, get_order(tableGroups));
				}
				dev->DC.calibrationInfo.tableVersion = tableVersion;				
				j = 16;
				for(i = 0; i < dev->DC.calibrationInfo.tableGroups; i++) {
					dev->DC.calibrationInfo.ptrIQtableEx[i].frequency = (pIQtableKernel[j+0] + pIQtableKernel[j+1]*256 
													+ pIQtableKernel[j+2]*256*256 + pIQtableKernel[j+3]*256*256*256);
					dev->DC.calibrationInfo.ptrIQtableEx[i].dAmp = (short)(pIQtableKernel[j+4] + pIQtableKernel[j+5]*256);
					dev->DC.calibrationInfo.ptrIQtableEx[i].dPhi = (short)(pIQtableKernel[j+6] + pIQtableKernel[j+7]*256);   
					j += 8;
				}
#if  0
				for(i = 0; i < dev->DC.calibrationInfo.tableGroups; i++) {
					 deb_data("[SETIQTable] frequency[%d] = %d\n",i, dev->DC.calibrationInfo.ptrIQtableEx[i].frequency);
					 deb_data("[SETIQTable] dAmp[%d] = %d\n",i, dev->DC.calibrationInfo.ptrIQtableEx[i].dAmp);
					 deb_data("[SETIQTable] dPhi[%d] = %d\n",i, dev->DC.calibrationInfo.ptrIQtableEx[i].dPhi);
				}
#endif
				pSetIQTableRequest->error = IT9507_setIQtable ((Modulator*) &(dev->DC.modulator), dev->DC.calibrationInfo);
				deb_data("[SETIQTable] Set IQtable group length is %d\n", dev->DC.calibrationInfo.tableGroups);						

				if(pIQtableKernel != NULL) {
					free_pages((unsigned long) pIQtableKernel, get_order(tableGroups*8+16));
					pIQtableKernel = NULL;
				}
			} else {
				dev->DC.calibrationInfo.ptrIQtableEx = NULL;
				pSetIQTableRequest->error = IT9507_setIQtable ((Modulator*) &(dev->DC.modulator), dev->DC.calibrationInfo);
				deb_data("[SETIQTable] Table NULL. Set default table.\n");
			}	
	
			return 0;
	}
	return DL_DemodIOCTLFun((void *)&dev->DC.modulator, (DWORD)cmd, parg);
}
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
long it950x_usb_tx_unlocked_ioctl(
	struct file *file,
	unsigned int cmd,
	unsigned long parg)
{
	struct it950x_dev	*dev;
    PDEVICE_CONTEXT		handle;
	SystemConfig		syscfg;
	PTxCmdRequest		pTxCmdRequest;
	TxSetDeviceTypeRequest pTxSetDeviceTypeRequest;
	TxGetDeviceTypeRequest pTxGetDeviceTypeRequest;
	PTxSetIQTableRequest 	pSetIQTableRequest;
	Byte *pIQtableUser, *pIQtableKernel;
	Word tableGroups, tableVersion;
	//Byte temp;
	int	 i, j;

	//deb_data("it951x_usb_tx_ioctl function\n");
	
	dev = (struct it950x_dev *)file->private_data;
	if (dev == NULL) {
		deb_data("dev is NULL\n");
		return -ENODEV;
	}

	handle = &dev->DC;
   /*
	* extract the type and number bitfields, and don't decode
	* wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	*/
	//if (_IOC_TYPE(cmd) != AFA_IOC_MAGIC) return -ENOTTY;
	//if (_IOC_NR(cmd) > AFA_IOC_MAXNR) return -ENOTTY;
    
	switch (cmd)
	{
		case IOCTL_ITE_MOD_STARTTRANSFER:
			tx_start_urb_transfer(dev);
			return 0;
					
		case IOCTL_ITE_MOD_STOPTRANSFER:
			tx_stop_urb_transfer(dev);
			return 0;
			
		case IOCTL_ITE_MOD_STARTTRANSFER_CMD:
#if RETURN_CHANNEL
			tx_start_urb_transfer_cmd(dev);
#endif			
			return 0;
			
		case IOCTL_ITE_MOD_STOPTRANSFER_CMD:
#if RETURN_CHANNEL		
			tx_stop_urb_transfer_cmd(dev);
#endif			
			return 0;
			
		case IOCTL_ITE_MOD_WRITE_CMD:
#if RETURN_CHANNEL
			pTxCmdRequest = (PTxCmdRequest) parg;
			Tx_RingBuffer_cmd(dev, (Byte*) pTxCmdRequest->cmdAddr, (Dword*) &pTxCmdRequest->len);
#endif
			return 0;
			
		case IOCTL_ITE_MOD_WRITE_LOWBITRATEDATA:
			SetLowBitRateTransfer(dev, (void*)parg);
			return 0;
			
		case IOCTL_ITE_MOD_GETDEVICETYPE:
		
			if (copy_from_user((void *)&pTxGetDeviceTypeRequest, (void *)parg, sizeof(TxGetDeviceTypeRequest)))
				return -EFAULT;

			if(handle->isUsingDefaultDeviceType == true){	// is Using Default devicetype. return default type and return error message.
				pTxGetDeviceTypeRequest.DeviceType = handle->deviceType;
				pTxGetDeviceTypeRequest.error = ModulatorError_INVALID_DEV_TYPE;
			} else {											// no using default. return current device type.
				pTxGetDeviceTypeRequest.DeviceType = handle->deviceType;			
				pTxGetDeviceTypeRequest.error = Error_NO_ERROR;			
			}
																																				                       			
			if (copy_to_user((void *)parg, (void *)&pTxGetDeviceTypeRequest, sizeof(TxGetDeviceTypeRequest)))
				return -EFAULT;
/*
			pTxGetDeviceTypeRequest = (PTxGetDeviceTypeRequest) parg;
			
			if(handle->isUsingDefaultDeviceType == true){	// is Using Default devicetype. return default type and return error message.
				pTxGetDeviceTypeRequest->DeviceType = handle->deviceType;
				pTxGetDeviceTypeRequest->error = ModulatorError_INVALID_DEV_TYPE;			
			} else {		// no using default. return current device type.
				pTxGetDeviceTypeRequest->DeviceType = handle->deviceType;			
				pTxGetDeviceTypeRequest->error = Error_NO_ERROR;			
			}*/
			return 0;
			
		case IOCTL_ITE_MOD_SETDEVICETYPE:
			if (copy_from_user((void *)&pTxSetDeviceTypeRequest, (void *)parg, sizeof(TxGetDeviceTypeRequest)))
				return -EFAULT;

			pTxSetDeviceTypeRequest.error = EagleUser_getSystemConfig((Modulator*) &(handle->modulator), pTxSetDeviceTypeRequest.DeviceType, &syscfg);
			if(pTxSetDeviceTypeRequest.error != Error_NO_ERROR)
				deb_data("EagleUser_getSystemConfig fail\n\t- DeviceType id: %d\n", pTxSetDeviceTypeRequest.DeviceType);
			else 
				deb_data("EagleUser_getSystemConfig ok\n\t- DeviceType id: %d\n", pTxSetDeviceTypeRequest.DeviceType);
			
			handle->isUsingDefaultDeviceType = false;	// Set for no default device type.
			handle->deviceType = pTxSetDeviceTypeRequest.DeviceType;
			handle->modulator.systemConfig = syscfg;

			pTxSetDeviceTypeRequest.error = EagleUser_setSystemConfig((Modulator*) &(handle->modulator), syscfg);
			if(pTxSetDeviceTypeRequest.error != Error_NO_ERROR)
				deb_data("EagleUser_setSystemConfig fail\n");	
			else 
				deb_data("EagleUser_setSystemConfig ok\n");			

			/* re-init GPIO setting for new device type */
			pTxSetDeviceTypeRequest.error = EagleUser_Initialization((Modulator*) &(handle->modulator));
			if(pTxSetDeviceTypeRequest.error != Error_NO_ERROR)		
				deb_data("EagleUser_Initialization fail\n");	
			else 														
				deb_data("EagleUser_Initialization ok\n");				

			if (copy_to_user((void *)parg, (void *)&pTxSetDeviceTypeRequest, sizeof(TxGetDeviceTypeRequest)))
				return -EFAULT;
			return 0;				

		case IOCTL_ITE_MOD_SETIQTABLE:
			pSetIQTableRequest = (PTxSetIQTableRequest) parg;
			pIQtableUser = (Byte*) pSetIQTableRequest->pIQtableAddr;

			if(pIQtableUser != NULL) {
				tableGroups = (Word) (pIQtableUser[14]<<8 | pIQtableUser[15]);
				tableVersion = (Dword) (pIQtableUser[10]<< 16 | pIQtableUser[11]<< 8 | pIQtableUser[12]);				
				if( (tableGroups*8+16) != pSetIQTableRequest->IQtableSize) {
						deb_data("[SETIQTable] Error: File size and table groups is different.\n");
						pSetIQTableRequest->error = ModulatorError_INVALID_DATA_LENGTH;				
						return 0;
				}

				pIQtableKernel = (Byte*) __get_free_pages(GFP_KERNEL, get_order(tableGroups*8+16));
				if (copy_from_user(pIQtableKernel, pIQtableUser, (tableGroups*8+16)))
					return -EFAULT;
				if(dev->DC.calibrationInfo.tableGroups != tableGroups) {		// Reallocate table.
					deb_data("[SETIQTable] Reallocate IQ Table.\n");
					if(dev->DC.calibrationInfo.ptrIQtableEx) {
						free_pages((unsigned long) dev->DC.calibrationInfo.ptrIQtableEx, get_order(dev->DC.calibrationInfo.tableGroups));
						dev->DC.calibrationInfo.ptrIQtableEx = NULL;
					}
					dev->DC.calibrationInfo.tableGroups = tableGroups;
					dev->DC.calibrationInfo.ptrIQtableEx = (IQtable*)__get_free_pages(GFP_KERNEL, get_order(tableGroups));
				}
				dev->DC.calibrationInfo.tableVersion = tableVersion;				
				j = 16;
				for(i = 0; i < dev->DC.calibrationInfo.tableGroups; i++) {
					dev->DC.calibrationInfo.ptrIQtableEx[i].frequency = (pIQtableKernel[j+0] + pIQtableKernel[j+1]*256 
													+ pIQtableKernel[j+2]*256*256 + pIQtableKernel[j+3]*256*256*256);
					dev->DC.calibrationInfo.ptrIQtableEx[i].dAmp = (short)(pIQtableKernel[j+4] + pIQtableKernel[j+5]*256);
					dev->DC.calibrationInfo.ptrIQtableEx[i].dPhi = (short)(pIQtableKernel[j+6] + pIQtableKernel[j+7]*256);   
					j += 8;
				}
#if  0
				for(i = 0; i < dev->DC.calibrationInfo.tableGroups; i++) {
					 deb_data("[SETIQTable] frequency[%d] = %d\n",i, dev->DC.calibrationInfo.ptrIQtableEx[i].frequency);
					 deb_data("[SETIQTable] dAmp[%d] = %d\n",i, dev->DC.calibrationInfo.ptrIQtableEx[i].dAmp);
					 deb_data("[SETIQTable] dPhi[%d] = %d\n",i, dev->DC.calibrationInfo.ptrIQtableEx[i].dPhi);
				}
#endif
				pSetIQTableRequest->error = IT9507_setIQtable ((Modulator*) &(dev->DC.modulator), dev->DC.calibrationInfo);
				deb_data("[SETIQTable] Set IQtable group length is %d\n", dev->DC.calibrationInfo.tableGroups);						

				if(pIQtableKernel != NULL) {
					free_pages((unsigned long) pIQtableKernel, get_order(tableGroups*8+16));
					pIQtableKernel = NULL;
				}
			} else {
				dev->DC.calibrationInfo.ptrIQtableEx = NULL;
				pSetIQTableRequest->error = IT9507_setIQtable ((Modulator*) &(dev->DC.modulator), dev->DC.calibrationInfo);
				deb_data("[SETIQTable] Table NULL. Set default table.\n");
			}	
	
			return 0;
	}
	return DL_DemodIOCTLFun((void *)&dev->DC.modulator, (DWORD)cmd, parg);

}
#endif

static ssize_t it950x_read(
	struct file *file, 
	char __user *buf,
	size_t count, 
	loff_t *ppos)
{
	struct it950x_dev *dev = (struct it950x_dev *)file->private_data;
	Dword Len = count;

	if (dev == NULL) 
		return -ENODEV;

	Rx_RingBuffer(dev, buf, &Len);
	//deb_data("ReadRingBuffer - %d\n", Len);

	return Len;

	/*AirHD*/
#if 0
	//udev = (struct usb_device *)file->private_data;
	//dev = (DEVICE_CONTEXT *)dev_get_drvdata(&udev->dev);

		ret = usb_bulk_msg(usb_get_dev(dev->DC->modulator.userData),
				usb_rcvbulkpipe(usb_get_dev(dev->DC->modulator.userData), 0x85), //dev->Demodulator.driver
				buffer,
				256,
				&nBytesRead,
				100000);	

		if (ret) deb_data("--------Usb2_readControlBus fail : 0x%08lx\n", ret);
	
		if (copy_to_user(buf, buffer, 256))
			return -EFAULT;

		for(i = 0; i < 256; i++)
			deb_data("---------Read data buffer[%d] 0x%x\n", i, buf[i]);

		return 0;
#endif

}

#ifdef TSDUCK_WRITE
static ssize_t it950x_tx_write_try(
	struct it950x_dev *dev,
	const char __user *user_buffer,
	size_t count,
	unsigned long flags)
{
	Dword Len = count;
	DWORD dwError = Error_NO_ERROR;
	spin_lock_irqsave(&dev->TxRBKeyLock, flags);
	dwError = Tx_RingBuffer(dev, (Byte*)user_buffer, &Len);
	spin_unlock_irqrestore(&dev->TxRBKeyLock, flags);
	return dwError;
}
#endif

static ssize_t it950x_tx_write(
	struct file *file,
	const char __user *user_buffer,
	size_t count, 
	loff_t *ppos)
{
	struct it950x_dev *dev;

	Dword Len = count;
	DWORD dwError = Error_NO_ERROR;
	unsigned long flags = 0;

	/*AirHD RingBuffer*/
	dev = (struct it950x_dev *)file->private_data;
	if (dev == NULL)
		return -ENODEV;

#ifdef TSDUCK_WRITE
        if (Len > dev->dwTxWriteTolBufferSize) {
            /* larger than urbn will never work */
            return Error_BUFFER_INSUFFICIENT;
        }
        /* wait and retry until buffer available (or other error) */
        wait_event_interruptible(dev->tx_urb_wait, (dwError = it950x_tx_write_try(dev, user_buffer, count, flags)) != Error_BUFFER_INSUFFICIENT);
#else
	spin_lock_irqsave(&dev->TxRBKeyLock, flags);
	dwError = Tx_RingBuffer(dev, (Byte*)user_buffer, &Len);
	spin_unlock_irqrestore(&dev->TxRBKeyLock, flags);
#endif
	return dwError;
	//deb_data("[%lu]\n", Len);
//	if(dwError != 0) return dwError;
//	else return Len;

	/*AirHD Bulk Msg*/
#if 0
	if (copy_from_user(b_buf, user_buffer, count))
		return -EFAULT;

	dwError = usb_bulk_msg(usb_get_dev(dev0->DC->modulator.userData),
			usb_sndbulkpipe(usb_get_dev(dev0->DC->modulator.userData), 0x06),
			b_buf,
			count,
			&act_len,
			100000);
   
	if(dev->is_use_low_brate) {
		//deb_data("it950x_tx_write: g_is_use_low_brate is open!\n");
		dwError = Tx_RingBuffer_low_brate(dev, user_buffer, &Len);
	}else{
		//deb_data("it950x_tx_write: g_is_use_low_brate is close!\n");
		dwError = Tx_RingBuffer(dev, user_buffer, &Len);
	}   
	if (dwError) deb_data("--------Usb2_writeControlBus fail : 0x%08lx\n", dwError);
	
	return count;
#endif
}


static struct file_operations it950x_usb_fops ={
	.owner =		THIS_MODULE,
	.open =		it950x_usb_open,
	.read =		it950x_read,
	//.write =	it950x_write,
	.release =	it950x_usb_release,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
		.unlocked_ioctl = it950x_usb_unlocked_ioctl,
#else
		.ioctl = it950x_usb_ioctl,
#endif

};

static struct file_operations it950x_usb_tx_fops ={
	.owner =		THIS_MODULE,
	.open =		it950x_usb_tx_open,
	//.read =		it950x_read,
	.write =	it950x_tx_write, /*AirHD*/ 
	.release =	it950x_usb_tx_release,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
		.unlocked_ioctl = it950x_usb_tx_unlocked_ioctl,
#else
		.ioctl = it950x_usb_tx_ioctl,
#endif

};

/*
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with devfs and the driver core
 */
static struct usb_class_driver it950x_class = {
//	.name =			"usb-it913x%d",
	.fops =			&it950x_usb_fops,
	.minor_base = 	USB_it913x_MINOR_BASE
};

static struct usb_class_driver it950x_class_tx = {
//	.name =			"usb-it950x%d",
	.fops =			&it950x_usb_tx_fops,
	.minor_base = 	USB_it950x_MINOR_BASE
};

static short usb_handle_set(Byte* usb_handles, Byte handle_size) 
{
	Byte i;

	for(i=0; i<handle_size; i++) {
		if(!usb_handles[i]) {
			usb_handles[i] = true;
			return i;
		}
	}
	deb_data("ERROR usb_handle_set\n");
	return -EBADF;
}

static short usb_handle_unset(Byte* usb_handles, Byte handle_unset_num) 
{
	if(usb_handles[handle_unset_num]) {
		usb_handles[handle_unset_num] = false;
		return 0;
	}
	deb_data("ERROR usb_handle_unset\n");
	return -EBADF;
}

#if 0
// Todo: would be used...
static DWORD ListAllHandleName()
{
	struct file * fi;
	struct dentry * thedentry;
	struct dentry * curdentry;
	const char * curname = NULL;
	DWORD dwError = Error_NO_ERROR;
	
	fi = filp_open("/dev/", O_RDONLY, 0);
	thedentry = fi->f_dentry;

	if(!IS_ERR(fi)) {
		list_for_each_entry(curdentry, &thedentry->d_subdirs, d_u.d_child) {
			curname = curdentry->d_name.name;
			printk(KERN_INFO "HandleName: %s \n", curname);
		}
	}
	filp_close(fi, NULL);
	
	return (dwError);
}
#endif
// Register a USB device node end
static int it950x_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct it950x_dev *dev = NULL;
	struct urb *purb;	
	int retval = -ENOMEM;
	int i;
	int order;
#if RETURN_CHANNEL	
	int order_cmd;	
#endif
	/* allocate memory for our device state and intialize it */
	dev = kzalloc(sizeof(struct it950x_dev), GFP_KERNEL);
	dev->g_AP_use = 0;
	atomic_set(&dev->g_AP_use_tx, 0);
	atomic_set(&dev->g_AP_use_rx, 0);	
	dev->DC.Tx_init_success = 0;
	dev->DC.Rx_init_success = 0;
	
	dev->DC.pControlBuffer_WriteReg = kzalloc(200, GFP_KERNEL);
	dev->DC.pControlBuffer_ReadReg = kzalloc(200, GFP_KERNEL);

	if (dev == NULL) {
		deb_data("Out of memory\n");
		return retval;
	}
	
	dev->usbdev = interface_to_usbdev(intf);

	/* Malloc IQ_TB */	
	dev->DC.calibrationInfo.ptrIQtableEx = 
		(IQtable*)__get_free_pages(GFP_KERNEL, get_order(IQ_TABLE_NROW));
	dev->DC.calibrationInfo.tableGroups = IQ_TABLE_NROW;
	dev->DC.calibrationInfo.tableVersion = 0;
	
	/* Malloc DC TB */
	dev->DC.dcInfo.tableGroups = 5;
	dev->DC.dcInfo.ptrDCtable = (DCtable*) kmalloc(sizeof(DCtable)*(dev->DC.dcInfo.tableGroups), GFP_KERNEL);
	dev->DC.dcInfo.ptrOFStable = (DCtable*) kmalloc(sizeof(DCtable)*(dev->DC.dcInfo.tableGroups), GFP_KERNEL);

	/* we can register the device now, as it is ready */
	usb_set_intfdata(intf, dev);
	deb_data("*** it950x usb device pluged in ***\n");
	retval = Device_init(interface_to_usbdev(intf), &dev->DC, true);
	if (retval) {
		deb_data("Device_init Fail: 0x%08x\n", retval);
		return retval;
	}

	/* Register Rx */
	if(dev->DC.Rx_init_success)	{
		/* Register Rx handle name */
		if(it950x_class.name == NULL) 
			it950x_class.name = kzalloc(sizeof(char)*MAX_USB_HANDLE_NAME_SIZE, GFP_KERNEL);
		dev->rx_handle_num = usb_handle_set(rx_usb_handles, RX_MAX_USB_HANDLE_NUM);
		if(dev->rx_handle_num >= 0)
		snprintf(it950x_class.name, MAX_USB_HANDLE_NAME_SIZE, "usb-it950x-rx%d", dev->rx_handle_num);
		else
			return -EBADF;

		/*  Register the device to usb core */
		retval = usb_register_dev(intf, &it950x_class);
		if (retval) {
			deb_data("Not able to get a minor for this device.");
			usb_set_intfdata(intf, NULL);
			//goto error;
			return -ENOTTY;
		}
		dev->rx_chip_minor = intf->minor;
		deb_data("rx minor %d \n", dev->rx_chip_minor);
	}

	/* Register Tx handle name */
	if(it950x_class_tx.name == NULL) 
		it950x_class_tx.name = kzalloc(sizeof(char)*MAX_USB_HANDLE_NAME_SIZE, GFP_KERNEL);
	dev->tx_handle_num = usb_handle_set(tx_usb_handles, TX_MAX_USB_HANDLE_NUM);
	if(dev->tx_handle_num >= 0)
		snprintf(it950x_class_tx.name, MAX_USB_HANDLE_NAME_SIZE, "usb-it950x%d", dev->tx_handle_num);
	else
		return -EBADF;

	/* Register Tx */
	intf->minor = -1;
	retval = usb_register_dev(intf, &it950x_class_tx);
	if (retval) {
		deb_data("Not able to get a minor for this device.");
		usb_set_intfdata(intf, NULL);
		//goto error;
		return -ENOTTY;
	}
	dev->tx_chip_minor = intf->minor;
	deb_data("tx minor %d \n", dev->tx_chip_minor);

        /* TSDuck patch, write(2) blocks until enough buffer is available */
#ifdef TSDUCK_WRITE
	init_waitqueue_head(&dev->tx_urb_wait);
#endif

	/* Allocate Write Ring buffer */
	dev->dwTxWriteTolBufferSize = URB_BUFSIZE_TX * URB_COUNT_TX;
	order = get_order(dev->dwTxWriteTolBufferSize);
	dev->pTxRingBuffer = (Byte*)__get_free_pages(GFP_KERNEL, order);
	if (dev->pTxRingBuffer) {
		dev->TxCurrBuffPointAddr = 0;
		dev->TxWriteBuffPointAddr = 0;
		dev->dwTxRemaingBufferSize = dev->dwTxWriteTolBufferSize;
		dev->tx_urb_index = 0;
	}

	/* Allocate Write low bitrate Ring buffer */
	dev->dwTxWriteTolBufferSize_low_brate = URB_BUFSIZE_TX_LOW_BRATE * URB_COUNT_TX_LOW_BRATE;
	order = get_order(dev->dwTxWriteTolBufferSize_low_brate);
	dev->pTxRingBuffer_low_brate = (Byte*)__get_free_pages(GFP_KERNEL, order);
	if (dev->pTxRingBuffer_low_brate) {
		dev->TxCurrBuffPointAddr_low_brate = 0;
		dev->TxWriteBuffPointAddr_low_brate = 0;
		dev->dwTxRemaingBufferSize_low_brate = dev->dwTxWriteTolBufferSize_low_brate;
		dev->tx_urb_index_low_brate = 0;
	}
#if RETURN_CHANNEL	
	/* Allocate Write cmd Ring buffer */
	dev->dwTxWriteTolBufferSize_cmd = URB_BUFSIZE_TX_CMD * URB_COUNT_TX_CMD;
	order_cmd = get_order(dev->dwTxWriteTolBufferSize_cmd + 8);
	dev->pTxRingBuffer_cmd = (Byte*)__get_free_pages(GFP_KERNEL, order_cmd);
	if (dev->pTxRingBuffer_cmd) {
		dev->pWriteFrameBuffer_cmd = dev->pTxRingBuffer_cmd + 8;
		dev->pTxCurrBuffPointAddr_cmd = (Dword*)dev->pTxRingBuffer_cmd;
		dev->pTxWriteBuffPointAddr_cmd = (Dword*)(dev->pTxRingBuffer_cmd + 4);
		dev->dwTxRemaingBufferSize_cmd = dev->dwTxWriteTolBufferSize_cmd;
		dev->tx_urb_index_cmd = 0;
		//dev->urb_use_count_cmd = URB_COUNT_TX_CMD;
	}
#endif	
	for (i = 0; i < URB_COUNT_TX; i++) {
		dev->tx_urbs[i] = usb_alloc_urb(0, GFP_KERNEL);
		if (!dev->tx_urbs[i])
			retval = -ENOMEM;		

		purb = dev->tx_urbs[i];
		purb->transfer_buffer = dev->pTxRingBuffer + (URB_BUFSIZE_TX*i);
		if (!purb->transfer_buffer) {
			usb_free_urb(purb);
			dev->tx_urbs[i] = NULL;
			deb_data("tx_urbs transfer_buffer alloc fail!\n");
			goto exit;
		}
		
		dev->tx_urb_context[i].index = i;
		dev->tx_urb_context[i].dev = dev;
		dev->tx_urbstatus[i] = 0;
		
		purb->status = -EINPROGRESS;
		usb_fill_bulk_urb(purb,
				  dev->usbdev,
				  usb_sndbulkpipe(dev->usbdev, 0x06),
				  purb->transfer_buffer,
				  URB_BUFSIZE_TX,
				  (usb_complete_t)tx_urb_completion,
				  &dev->tx_urb_context[i]);
		
		purb->transfer_flags = 0;
	}
	
	for (i = 0; i < URB_COUNT_TX_LOW_BRATE; i++) {
		dev->tx_urbs_low_brate[i] = usb_alloc_urb(0, GFP_KERNEL);
		if (!dev->tx_urbs_low_brate[i])
			retval = -ENOMEM;		

		purb = dev->tx_urbs_low_brate[i];
		purb->transfer_buffer = dev->pTxRingBuffer_low_brate + (URB_BUFSIZE_TX_LOW_BRATE*i);
		if (!purb->transfer_buffer) {
			usb_free_urb(purb);
			dev->tx_urbs_low_brate[i] = NULL;
			deb_data("tx_urbs_low_brate transfer_buffer alloc fail!\n");
			goto exit;
		}
		
		dev->tx_urb_context_low_brate[i].index = i;
		dev->tx_urb_context_low_brate[i].dev = dev;
		dev->tx_urbstatus_low_brate[i] = 0;
		purb->status = -EINPROGRESS;
		usb_fill_bulk_urb(purb,
				  dev->usbdev,
				  usb_sndbulkpipe(dev->usbdev, 0x06),
				  purb->transfer_buffer,
				  URB_BUFSIZE_TX_LOW_BRATE,
				  (usb_complete_t)tx_urb_completion_low_brate,
				  &dev->tx_urb_context_low_brate[i]);
		purb->transfer_flags = 0;
	}		
#if RETURN_CHANNEL
	for (i = 0; i < URB_COUNT_TX_CMD; i++) {
		dev->tx_urbs_cmd[i] = usb_alloc_urb(0, GFP_KERNEL);
		if (!dev->tx_urbs_cmd[i])
			retval = -ENOMEM;			

		purb = dev->tx_urbs_cmd[i];

		purb->transfer_buffer = dev->pWriteFrameBuffer_cmd + (URB_BUFSIZE_TX_CMD*i);
		if (!purb->transfer_buffer) {
			usb_free_urb(purb);
			dev->tx_urbs_cmd[i] = NULL;
			goto exit;
		}
		
		dev->tx_urb_context_cmd[i].index = i;
		dev->tx_urb_context_cmd[i].dev = dev;
		dev->tx_urbstatus_cmd[i] = 0;
		
		purb->status = -EINPROGRESS;
		usb_fill_bulk_urb(purb,
				  dev->usbdev,
				  usb_sndbulkpipe(dev->usbdev, 0x06),
				  purb->transfer_buffer,
				  URB_BUFSIZE_TX_CMD,
				  (usb_complete_t)tx_urb_completion_cmd,
				  &dev->tx_urb_context_cmd[i]);
		
		purb->transfer_flags = 0;
	}
#endif
	/* Rx Ring buffer alloc & chip init */
	spin_lock_init(&dev->RxRBKeylock);
	dev->rx_urb_streaming = 0;
	dev->dwRxReadRemaingBufferSize = 0;
	dev->dwRxTolBufferSize = URB_BUFSIZE_RX * URB_COUNT_RX;
	dev->rx_urb_index = 0;
	dev->rx_first_urb_reset = 0;
	
	atomic_set(&dev->rx_urb_counter, URB_COUNT_RX);
	dev->pRxRingBuffer = (Byte*)__get_free_pages(GFP_KERNEL, get_order(dev->dwRxTolBufferSize));

	/*Allocate RX urb*/	
	for (i = 0; i < URB_COUNT_RX; i++) {
		dev->rx_urbs[i] = usb_alloc_urb(0, GFP_KERNEL);
		if (!dev->rx_urbs[i])
			retval = -ENOMEM;
			
		dev->rx_urbs[i]->transfer_buffer = dev->pRxRingBuffer + (URB_BUFSIZE_RX * i);

		if(!dev->rx_urbs[i]->transfer_buffer){
			usb_free_urb(dev->rx_urbs[i]);
			dev->rx_urbs[i] = NULL;
			retval = -ENOMEM;
			goto exit;
		}

		dev->rx_urb_context[i].index = i;
		dev->rx_urbstatus[i] = 0;
		dev->rx_urb_context[i].dev = dev;
		dev->rx_urbs[i]->status = -EINPROGRESS;
		
		usb_fill_bulk_urb(dev->rx_urbs[i],
				  dev->usbdev,
				  usb_rcvbulkpipe(dev->usbdev, 0x85),
				  dev->rx_urbs[i]->transfer_buffer,
				  URB_BUFSIZE_RX,
				  (usb_complete_t)rx_urb_completion,
				  &dev->rx_urb_context[i]);
						  
			dev->rx_urbs[i]->transfer_flags = 0;

	}	
#if PACKECT_SHIFT
	/* Init reset buffer thread */
	dev->buffer_shift = 0;	
	dev->reset_thread = kthread_create(&Thread_buffer_reset, (void *)dev, "Reset_Buffer_Thread");
	if(!dev->reset_thread || IS_ERR(dev->reset_thread)){
		deb_data("\t Error: Reset_Buffer_Thread fail\n");
		retval = PTR_ERR(dev->reset_thread);
		dev->reset_thread = NULL;
		goto exit;
	}
	wake_up_process(dev->reset_thread);	
#endif

exit:	
#if (DEVICE_POWER_CTRL)
	atomic_set(&dev->tx_pw_on, 0);	
	atomic_set(&dev->rx_pw_on, 0);	
	DL_ApPwCtrl(&dev->DC, 1, 0);	
	DL_ApPwCtrl(&dev->DC, 0, 0);	
#endif	
	deb_data("USB ITEtech device now attached to USBSkel-%d \n", intf->minor);

	return retval;
}

static int it950x_suspend(struct usb_interface *intf, pm_message_t state)
{
	int error;
	struct it950x_dev *dev;

	dev = usb_get_intfdata(intf);
	if (!dev) 
		deb_data("dev = NULL");
	//deb_data("Enter %s Function\n", __FUNCTION__);

#ifdef EEEPC
	error = DL_Reboot();
#else
    if (dev->DevicePower) {
		error = DL_CheckTunerInited(&dev->DC, 0, &dev->TunerInited0);
		error = DL_CheckTunerInited(&dev->DC, 1, &dev->TunerInited1);

		error = DL_ApCtrl(&dev->DC, 0, 0);
		error = DL_ApCtrl(&dev->DC, 1, 0);
		if (error) {
			deb_data("DL_ApCtrl error : 0x%x\n", error);
		}

		dev->DeviceReboot = true;
    }
#endif
	
	return 0;
}

static int it950x_resume(struct usb_interface *intf)
{
	int retval = -ENOMEM;
	int error;
	struct it950x_dev *dev;

	//deb_data("Enter %s Function\n",__FUNCTION__);
	dev = usb_get_intfdata(intf);
	if (!dev) 
		deb_data("dev = NULL");


#ifdef EEEPC
#else
    if (dev->DeviceReboot == True) {
		retval = Device_init(interface_to_usbdev(intf),&dev->DC, false);
		if(retval)
			deb_data("Device_init Fail: 0x%08x\n", retval);

		if (dev->TunerInited0)
			error = DL_ApCtrl(&dev->DC, 0, 1);
		if (dev->TunerInited1)
			error = DL_ApCtrl(&dev->DC, 1, 1);
    }
#endif
	
    return 0;
}

static void it950x_disconnect(struct usb_interface *intf)
{
	struct it950x_dev *dev;
	int minor = intf->minor;
	int order, i;	

	deb_data("%s()\n", __func__);
	/* prevent afa_open() from racing afa_disconnect() */
	//lock_kernel();
//	mutex_lock(&it950x_mutex);

	dev = usb_get_intfdata(intf);
	if (!dev) {
		deb_data("dev = NULL");
	}
	
	/*DM368 usb bus error when using kill urb */
#if 0	
	mutex_lock(&it950x_urb_kill);
	
	if(dev->tx_on) tx_kill_busy_urbs(dev);
	if(dev->rx_on) rx_kill_busy_urbs(dev);
	
	mutex_unlock(&it950x_urb_kill);
#endif

	/* Release RBRingBuffer and urbs */
	tx_free_urbs(dev);		
	if (dev->pTxRingBuffer) {
		order = get_order(dev->dwTxWriteTolBufferSize);
		free_pages((long unsigned int)dev->pTxRingBuffer, order);
	}
#if RETURN_CHANNEL
	if (dev->pTxRingBuffer_cmd) {
		order = get_order(dev->dwTxWriteTolBufferSize_cmd + 8);
		free_pages((long unsigned int)dev->pTxRingBuffer_cmd, order);
	}
#endif
	if (dev->pTxRingBuffer_low_brate) {
		order = get_order(dev->dwTxWriteTolBufferSize_low_brate);
		free_pages((long unsigned int)dev->pTxRingBuffer_low_brate, order);
	}			
	if (dev->pRxRingBuffer)
		free_pages((long unsigned int)dev->pRxRingBuffer, get_order(dev->dwRxTolBufferSize));

	/* if urb doesn't call back, kill it. */
	for (i = 0; i < URB_COUNT_RX; i++) {   
		if(dev->rx_urbstatus[i] == 1){
			usb_kill_urb(dev->rx_urbs[i]);
		}
	}	
	rx_free_urbs(dev);	
	
#if PACKECT_SHIFT
	dev->disconnect = 1;
#endif
	/*DM368 usb bus error when using kill urb */
#if 0	
	mutex_lock(&it951x_urb_kill);
	
	if(dev->tx_on) tx_kill_busy_urbs(dev);
	if(dev->rx_on) rx_kill_busy_urbs(dev);
	
	mutex_unlock(&it951x_urb_kill);
#endif

	usb_set_intfdata(intf, NULL);

	/* give back our minor */
	/* Rx de-register */
	if(dev->DC.Rx_init_success)	{	
		intf->minor = dev->rx_chip_minor;
		usb_deregister_dev(intf, &it950x_class);
		usb_handle_unset(rx_usb_handles, dev->rx_handle_num);
	}
	
	/* Tx de-register */
	intf->minor = dev->tx_chip_minor;
	usb_deregister_dev(intf, &it950x_class_tx);
	usb_handle_unset(tx_usb_handles, dev->tx_handle_num);

	/* Free IQ_TB */
	if(dev->DC.calibrationInfo.ptrIQtableEx) {
		free_pages((unsigned long) dev->DC.calibrationInfo.ptrIQtableEx, get_order(dev->DC.calibrationInfo.tableGroups));
		dev->DC.calibrationInfo.ptrIQtableEx = NULL;
	}
	if(dev->DC.dcInfo.ptrDCtable) 
		kfree((void*)dev->DC.dcInfo.ptrDCtable);
	if(dev->DC.dcInfo.ptrOFStable) 
		kfree((void*)dev->DC.dcInfo.ptrOFStable);

//	mutex_unlock(&it950x_mutex);
	//unlock_kernel();
	
	/* decrement our usage count */
	//	kref_put(&dev->kref, afa_delete);

	if (dev){
		if(dev->file) dev->file->private_data = NULL;
		if(dev->tx_file) dev->tx_file->private_data = NULL;
		if(dev->DC.pControlBuffer_WriteReg) { 	/* Malloc Usb2_writeControlBus Write Buffer */
			kfree(dev->DC.pControlBuffer_WriteReg);
			dev->DC.pControlBuffer_WriteReg = NULL;
		}
		if(dev->DC.pControlBuffer_ReadReg) { 	/* Malloc Usb2_writeControlBus Read Buffer */
			kfree(dev->DC.pControlBuffer_ReadReg);
			dev->DC.pControlBuffer_ReadReg = NULL;
		}
		kfree(dev);
	}
	
	deb_data("USB ITEtech #%d now disconnected", minor);
}


static struct usb_driver it950x_driver = {
#if LINUX_VERSION_CODE <=  KERNEL_VERSION(2,6,15)
	.owner = THIS_MODULE,
#endif
	.name       = "usb-it950x",
	.probe      = it950x_probe,
	.disconnect = it950x_disconnect,//dvb_usb_device_exit,
	.id_table   = it950x_usb_id_table,
	.suspend    = it950x_suspend,
	.resume     = it950x_resume,
};

static int __init it950x_module_init(void)
{
	int result;

	//info("%s",__FUNCTION__);
	//deb_data("usb_it950x Module is loaded \n");

	if ((result = usb_register(&it950x_driver))) {
		pr_err("usb_register failed. Error number %d",result);
		return result;
	}
	
	return 0;
}

static void __exit it950x_module_exit(void)
{
	deb_data("usb_it950x Module is unloaded!\n");
	usb_deregister(&it950x_driver);
}

module_init (it950x_module_init);
module_exit (it950x_module_exit);

MODULE_AUTHOR("Jason Dong <jason.dong@ite.com.tw>");
MODULE_DESCRIPTION("DTV Native Driver for Device Based on ITEtech it950x");
MODULE_VERSION(DRIVER_RELEASE_VERSION);
MODULE_LICENSE("GPL");
