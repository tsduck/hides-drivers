## Device drivers for HiDes UT-100C DVB-T Modulator

### About HiDes devices

[HiDes](http://www.hides.com.tw/) is a company from Taiwan, a manufacturer
of cheap DVB-T devices. These devices are based on chips from
[ITE Technologies Inc.](http://www.ite.com.tw/), also from Taiwan.

The UT-100C model is a USB DVB-T modulator adaptor (transmission).
This device is probably the cheapest modulator on Earth for Digital TV.
Other models include reception or PCIe interface.

The associated software, as currently delivered by HiDes, supports Windows
and Linux. It is currently unclear who actually developed this software,
HiDes or ITE. 

The software is poorly packaged and its delivery mostly consists in a
set of disorganized zip archives on DropBox, without fixed URL to reference
in order to get notified of updates.

### About this project

This project proposes an integrated packaging of the drivers for HiDes devices
on Linux and Windows.

The latest repackaged drivers are available [here](https://github.com/tsduck/hides-drivers/releases/latest).

Synthetic links to the latest versions are also available from the
[TSDuck site](https://tsduck.io/download/hides/).

#### Windows drivers

The original packaging for the Windows driver for HiDes devices is acceptable.
No change was made. This project only provides a fixed and known place from
where the latest version can be downloaded.

#### Linux drivers

The original Linux driver has two problems:
- It is delivered in source form only, in the middle of an archive containing
sample test code and other software.
- It has a _polling_ design for send operations. A "normal" device driver for
output device implements "waiting" or "blocking" send operations.

To solve the first issue, a DKMS (Dynamic Kernel Module Support) package is
provided in this project. Install the DMKS package (available in `.rpm` or
`.deb` form) and the driver is automatically recompiled and reinstalled after
each kernel upgrade.

To solve the second issue, the Linux driver was modified to make all `write`
operations waiting for the device to be ready. Thus, applications do not
have to implement polling.

Note that the modified driver remains otherwise compatible with the original
one. Thus, applications which were developed for the original driver will
continue to work. But their polling code will never be used since all `write`
calls always succeed the first time (except in case of _real_ errors of course).

To let applications checking if the driver needs polling or not, the version
string of the new driver ends with a `'w'` letter.

#### API and SDK

The original SDK is hopeless. The API definition is not even the same between
Linux and Windows. It is not possible to write portable applications using
the original SDK. Moreover, the Linux version of the SDK exhibits blatant
memory leaks in its so-called "API". Using the original SDK is consequently
discouraged.

An alternate portable HiDes API is provided as part of [TSDuck](https://github.com/tsduck/tsduck/).
This API is the C++ class [HiDesDevice](https://github.com/tsduck/tsduck/blob/master/src/libtsduck/tsHiDesDevice.h).
The interface of this class is system-agnostic. Its implementation directly
calls the underlying device driver but hides the differences between operating systems.

### Reference links

- [UT-100 USB DVB-T modulator adaptor product page](http://www.hides.com.tw/product_cg74469_eng.html)
- [Latest software downlinks](http://www.hides.com.tw/downloads_eng.html)
  (browse to _"Peripheral Transceiver / Transmitter / Receiver"_, table _"DVB-T"_,
  column _"Transmitter"_, then _"UT-100C"_)

### Latest links

- [Latest release](https://www.dropbox.com/sh/zcrqdf5xrvfa4wz/AACib5Z7EBErqk35N-DZGt5Pa)
- [Original Windows drivers](https://www.dropbox.com/sh/zcrqdf5xrvfa4wz/AABygNsKZCQ2xdjnpmMb9oNca/Driver_Windows_15.11.27.1.zip?dl=1)
- [Original Linux drivers and API sources](https://www.dropbox.com/sh/zcrqdf5xrvfa4wz/AACnUvOIG4OFG26w2uSfvuuDa/it950x_linux_v16.11.10.1.zip?dl=1)
- [Full environment](https://www.dropbox.com/sh/zcrqdf5xrvfa4wz/AACjjPV-gdoWeVhvyys7W28Ua/UT-100%20CD%20v2.4_20161003_withDEC.zip?dl=1)
