/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 */

#define K_ESC -0x06
#define K_BACKSPACE -0x07
#define K_TABS -0x08
#define K_ENTER -0x09
#define K_CAPSLOCK -0x0A
#define K_SYSTEM -0x0B

#define K_SHIFT_L -0x10
#define K_SHIFT_R -0x11
#define K_CTRL_L -0x12
#define K_CTRL_R -0x13
#define K_ALT -0x14
#define K_ALT_GR -0x15

#define K_INSERT -0x20
#define K_DELETE -0x21
#define K_HOME -0x22
#define K_END -0x23
#define K_PAGE_UP -0x24
#define K_PAGE_DOWN -0x25
#define K_ARROW_LEFT -0x26
#define K_ARROW_RIGHT -0x27
#define K_ARROW_UP -0x28
#define K_ARROW_DOWN -0x29

#define K_NUM_0 -0x30
#define K_NUM_1 -0x31
#define K_NUM_2 -0x32
#define K_NUM_3 -0x33
#define K_NUM_4 -0x34
#define K_NUM_5 -0x35
#define K_NUM_6 -0x36
#define K_NUM_7 -0x37
#define K_NUM_8 -0x38
#define K_NUM_9 -0x39
#define K_NUM_DOT -0x3A
#define K_NUM_ADD -0x3B
#define K_NUM_SUB -0x3C
#define K_NUM_MUL -0x3D
#define K_NUM_DIV -0x3E
#define K_NUM_ENTER -0x3F
#define K_NUM_LOCK -0x40

#define K_F1 -0x41
#define K_F2 -0x42
#define K_F3 -0x43
#define K_F4 -0x44
#define K_F5 -0x45
#define K_F6 -0x46
#define K_F7 -0x47
#define K_F8 -0x48
#define K_F9 -0x49
#define K_F10 -0x4A
#define K_F11 -0x4B
#define K_F12 -0x4C

extern int keyboard_layout_US[0x90][4];
