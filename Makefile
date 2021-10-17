#      This file is part of the KoraOS project.
#  Copyright (C) 2015-2021  <Fabien Bavent>
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

include $(topdir)/make/build.mk
include $(topdir)/make/check.mk
# include $(topdir)/make/targets.mk

CFLAGS ?= -Wall -Wextra -ggdb

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

DISTO ?= lgfx

CFLAGS_g += $(CFLAGS) -I$(topdir)/include -fPIC

ifneq ($(sysdir),)
CFLAGS_g += -I$(sysdir)/include
LFLAGS_g += -L$(sysdir)/lib
endif


SRCS_g += $(wildcard $(srcdir)/core/*.c)
SRCS_g += $(wildcard $(srcdir)/utils/*.c)
# SRCS_g += $(wildcard $(srcdir)/widgets/*.c)
SRCS_g += $(srcdir)/$(DISTO).c

LFLAGS_g += -lgfx -lpng -lm -lz
#  -lcairo

$(eval $(call comp_source,g,CFLAGS_g))
$(eval $(call link_shared,gum,SRCS_g,LFLAGS_g,g))


SRCS_l += $(srcdir)/tests/logon.c
LFLAGS_l += $(LFLAGS_g) -L$(libdir) -lgum
$(eval $(call link_bin,logon,SRCS_l,LFLAGS_l,g))


SRCS_w += $(srcdir)/tests/widgets.c
LFLAGS_w += $(LFLAGS_g) -L $(libdir) -lgum
$(eval $(call link_bin,widgets,SRCS_w,LFLAGS_w,g))


# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

ifeq ($(NODEPS),)
-include $(call fn_deps,SRCS_g,g)
-include $(call fn_deps,SRCS_l,g)
-include $(call fn_deps,SRCS_w,g)
endif
