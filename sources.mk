#      This file is part of the SmokeOS project.
#  Copyright (C) 2015  <Fabien Bavent>
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
NAME = Gum
VERSION := $(GIT_DESC)
DISTO ?= cairo

# F L A G S -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
CFLAGS += -Wall -Wextra -Wno-unused-parameter -fPIC -Wno-multichar
CFLAGS += -I$(topdir)/include
# CFLAGS += -D__GUM_X11

# LFLAGS += -L/usr/X11R6/lib -lX11
ifeq ($(DISTO),cairo)
LFLAGS += -L/usr/X11R6/lib -L/usr/include/cairo -lX11 -lcairo -lrt
endif


# C O M P I L E   M O D E -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
std_CFLAGS := $(CFLAGS) -ggdb
$(eval $(call ccpl,std))


# D E L I V E R I E S -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
gum_src-y += $(wildcard $(srcdir)/utils/*.c)
gum_src-y += $(wildcard $(srcdir)/core/*.c)
gum_src-y += $(srcdir)/$(DISTO).c
gum_LFLAGS := $(LFLAGS)
$(eval $(call llib,gum,std))
DV_LIBS += $(libdir)/libgum.so

brws_src-y += $(gum_src-y)
brws_src-y += $(srcdir)/tests/browser.c
brws_LFLAGS := $(LFLAGS)
$(eval $(call link,brws,std))
DV_UTILS += $(bindir)/brws

logon_src-y += $(gum_src-y)
logon_src-y += $(srcdir)/tests/logon.c
logon_LFLAGS := $(LFLAGS)
$(eval $(call link,logon,std))
DV_UTILS += $(bindir)/logon

