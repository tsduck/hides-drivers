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
HiDes or ITE. However, it is poorly packaged, especially the Linux drivers and
the application SDK. The software delivery mostly consists in a set of
disorganized zip archives on DropBox, without fixed URL to reference
in order to get notified of updates.

This project proposes an integrated packaging of the drivers for HiDes devices
on Linux and Windows. When the original packaging is acceptable (as it is for
the Windows drivers, for instance), no change was made. For Linux drivers,
a DKMS (Dynamic Kernel Module Support) package is provided.

The latest repackaged drivers are available [here](https://github.com/tsduck/hides-drivers/releases/latest).

The provided SDK, however, is hopeless. The API definition is not even the same
between Linux and Windows. An alternate portable HiDes API will be provided as
part of [TSDuck](https://github.com/tsduck/tsduck/), based on the original
HiDes / ITE drivers.

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
