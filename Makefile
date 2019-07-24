#      This file is part of the KoraOS project.
#  Copyright (C) 2018  <Fabien Bavent>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Affero General Public License as
#  published by the Free Software Foundation, either version 3 of the
#  License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Affero General Public License for more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
topdir ?= $(shell readlink -f $(dir $(word 1,$(MAKEFILE_LIST))))
gendir ?= $(shell pwd)

include $(topdir)/make/global.mk
srcdir = $(topdir)/src

all: libgum
install: $(prefix)/lib/libgum.so


DISTO ?= cairo

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

CFLAGS ?= -Wall -Wextra -Wno-unused-parameter  -ggdb
CFLAGS += -I$(topdir)/include
CFLAGS += -fPIC

include $(topdir)/make/build.mk

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

SRCS-y += $(wildcard $(srcdir)/core/*.c)
SRCS-y += $(wildcard $(srcdir)/utils/*.c)
SRCS-y += $(wildcard $(srcdir)/widgets/*.c)
SRCS-y += $(srcdir)/$(DISTO).c

ifeq ($(DISTO),kora)
endif


$(eval $(call link_shared,gum,SRCS,LFLAGS))

logon_SRCS-y += $(srcdir)/tests/logon.c
logon_LFLGS += -L $(libdir) -lgum -lcairo
$(eval $(call link_bin,logon,logon_SRCS,logon_LFLGS))


ifeq ($(NODEPS),)
# -include $(call fn_deps,SRCS-y)
endif

