##
## This file is part of the libsigrok project.
##
## Copyright (C) 2011 Uwe Hermann <uwe@hermann-uwe.de>
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.
##

SUBDIRS = dmm

# Local lib, this is NOT meant to be installed!
noinst_LTLIBRARIES = libsigrok_hw_common.la

libsigrok_hw_common_la_SOURCES = scpi.c scpi_tcp.c scpi_usbtmc.c

if NEED_RPC
libsigrok_hw_common_la_SOURCES += scpi_vxi.c vxi_clnt.c vxi_xdr.c
endif

if NEED_SERIAL
libsigrok_hw_common_la_SOURCES += serial.c scpi_serial.c
endif

if NEED_USB
libsigrok_hw_common_la_SOURCES += ezusb.c usb.c scpi_usbtmc_libusb.c
endif

if NEED_VISA
libsigrok_hw_common_la_SOURCES += scpi_visa.c
endif

libsigrok_hw_common_la_LIBADD = dmm/libsigrok_hw_common_dmm.la

libsigrok_hw_common_la_CFLAGS = \
	-I$(top_srcdir)

