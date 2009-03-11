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
../Src_AM/softuart.c \
../Src_AM/main.c \
../Src_AM/output.c \
../Src_common/Platform_AVR/hal_atmel.c \
../Src_common/msg.c \
../Src_common/phc.c \
../Src_common/timer.c \
../Src_common/uart.c \
../Src_common/random.c \
../Src_common/errlog.c

# Place -D or -U options here for C sources
CDEFS = -DCFG_OUTPUT
#-DCFG_ADC

# now call the big common makefile part
include ../Src_common/Platform_AVR/makefile.common
