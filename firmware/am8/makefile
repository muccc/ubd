##############################################################################
#
# OpenHC:                          ___                 _  _  ___
#  Open source                    / _ \ _ __  ___ _ _ | || |/ __|
#  Home                          | (_) | '_ \/ -_) ' \| __ | (__ 
#  Control                        \___/| .__/\___|_||_|_||_|\___|
# http://openhc.sourceforge.net/       |_| 
#
# Copyright (C) 2005 by Joerg Hohensohn
#
# All files in this archive are subject to the GNU General Public License.
# See http://www.gnu.org/licenses/gpl-3.0.txt for full license agreement.
#
# This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
# KIND, either express or implied.
#
##############################################################################

####### The variable part of the makefile #######

# MCU name
MCU = atmega8
#MCU = atmega16
#MCU = atmega88

# Target file name (without extension).
TARGET = am

# List C source files here. (C dependencies are automatically generated.)
SRC = \
../output/softuart.c \
../output/main.c \
../output/output.c \
../lib/Platform_AVR/hal_atmel.c \
../lib/msg.c \
../lib/phc.c \
../lib/timer.c \
../lib/uart.c \
../lib/random.c \
../lib/errlog.c

# Place -D or -U options here for C sources
CDEFS = -DCFG_OUTPUT
#-DCFG_ADC

# now call the big common makefile part
include ../lib/Platform_AVR/makefile.common
