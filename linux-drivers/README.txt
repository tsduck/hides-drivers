This directory contains the HiDes Linux drivers.

The directory it950x_driver contains the original source files for
the driver, comming from ITE, referenced by HiDes software page.
These files were slightly modified to compile on kernel versions
ranging from 3.10 (RedHat / CentOS 7.5) to 4.16 (Fedora 28).

The original downloaded source archive can be found in ../downloads.

The command 'make' creates a DKMS package with the driver source files.
When installed, the DKMS package automatically recompiles the driver
each time the kernel is upgraded.

Running from RedHat/CentOS/Fedora, a .rpm package is created.
Running from Ubuntu/Debian/Raspbian, a .deb package is created.
