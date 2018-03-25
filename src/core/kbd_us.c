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
#include <kora/keys.h>

#define _K4(n) n, n, n, n
#define _K3(n) n, n, n
#define _K2(n) n, n

int keyboard_layout_US[0x90][4] =
{
  { _K4(0) }, // 00
  { _K4(0) }, // 01
  { _K4(0) }, // 02
  { _K4(0) }, // 03
  { _K4(0) }, // 04
  { _K4(0) }, // 05
  { _K4(0) }, // 06
  { _K4(0) }, // 07
  { _K4(0) }, // 08
  { _K4(K_ESC) }, // 09
  { '1', '!', '¡', '¹' }, // 0A
  { '2', '@', '²', 0 }, // 0B
  { '3', '#', '³',  }, // 0C
  { '4', '$', '¤', '£' }, // 0D
  { '5', '%', '€', 0 }, // 0E
  { '6', '^', '¼', 0 }, // 0F
  { '7', '&', '½', 0 }, // 10
  { '8', '*', '¾', 0 }, // 11
  { '9', '(', '‘', 0 }, // 12
  { '0', ')', '’', 0 }, // 13
  { '-', '_', '¥', 0 }, // 14
  { '=', '+', '×', '÷' }, // 15
  { _K4(K_BACKSPACE) }, // 16
  { _K4(K_TABS) }, // 17
  { 'q', 'Q', 'ä', 'Ä' }, // 18
  { 'w', 'W', 'å', 'Å' }, // 19
  { 'e', 'E', 'é', 'É' }, // 1A
  { 'r', 'R', '®', 0 }, // 1B
  { 't', 'T', 'þ', 'Þ' }, // 1C
  { 'y', 'Y', 'ü', 'Ü' }, // 1D
  { 'u', 'U', 'ú', 'Ú' }, // 1E
  { 'i', 'I', 'í', 'Í' }, // 1F
  { 'o', 'O', 'ó', 'Ó' }, // 20
  { 'p', 'P', 'ö', 'Ö' }, // 21
  { '[', '{', '«', 0 }, // 22
  { ']', '}', '»', 0 }, // 23
  { _K4(K_ENTER) }, // 24
  { _K4(K_CTRL_L) }, // 25
  { 'a', 'A', 'á', 'Á' }, // 26
  { 's', 'S', 'ß', '§' }, // 27
  { 'd', 'D', 'ð', 'Ð' }, // 28
  { 'f', 'F', _K2(0) }, // 29
  { 'g', 'G', _K2(0) }, // 2A
  { 'h', 'H', _K2(0) }, // 2B
  { 'j', 'J', _K2(0) }, // 2C
  { 'k', 'K', _K2(0) }, // 2D
  { 'l', 'L', 'ø', 'Ø' }, // 2E
  { ';', ':', '¶', '°' }, // 2F
  { '\'', '"', '´', '¨' }, // 30
  { '`', '~', _K2(0) }, // 31
  { _K4(K_SHIFT_L) }, // 32
  { '\'', '|', '¬', '¦' }, // 33
  { 'z', 'Z', 'æ', 'Æ' }, // 34
  { 'x', 'X', _K2(0) }, // 35
  { 'c', 'C', '©', '¢' }, // 36
  { 'v', 'V', _K2(0) }, // 37
  { 'b', 'B', _K2(0) }, // 38
  { 'n', 'N', 'ñ', 'Ñ' }, // 39
  { 'm', 'M', 'µ', 0 }, // 3A
  { ',', '<', 'ç', 'Ç' }, // 3B
  { '.', '>', _K2(0) }, // 3C
  { '/', '?', '¿', 'Ä' }, // 3D
  { _K4(K_SHIFT_R) }, // 3E
  { _K4(K_NUM_MUL) }, // 3F
  { _K4(K_ALT) }, // 40
  { _K4(' ') }, // 41
  { _K4(K_CAPSLOCK) }, // 42
  { _K4(K_F1) }, // 43
  { _K4(K_F2) }, // 44
  { _K4(K_F3) }, // 45
  { _K4(K_F4) }, // 46
  { _K4(K_F5) }, // 47
  { _K4(K_F6) }, // 48
  { _K4(K_F7) }, // 49
  { _K4(K_F8) }, // 4A
  { _K4(K_F9) }, // 4B
  { _K4(K_F10) }, // 4C
  { _K4(K_NUM_LOCK) }, // 4D
  { _K4(0) }, // 4E
  { _K4(K_NUM_7) }, // 4F
  { _K4(K_NUM_8) }, // 50
  { _K4(K_NUM_9) }, // 51
  { _K4(K_NUM_SUB) }, // 52
  { _K4(K_NUM_4) }, // 53
  { _K4(K_NUM_5) }, // 54
  { _K4(K_NUM_6) }, // 55
  { _K4(K_NUM_ADD) }, // 56
  { _K4(K_NUM_1) }, // 57
  { _K4(K_NUM_2) }, // 58
  { _K4(K_NUM_3) }, // 59
  { _K4(K_NUM_0) }, // 5A
  { _K4(K_NUM_DOT) }, // 5B
  { _K4(0) }, // 5C
  { _K4(0) }, // 5D
  { _K4(0) }, // 5E
  { _K4(K_F11) }, // 5F
  { _K4(K_F12) }, // 60
  { _K4(0) }, // 61
  { _K4(0) }, // 62
  { _K4(0) }, // 63
  { _K4(0) }, // 64
  { _K4(0) }, // 65
  { _K4(0) }, // 66
  { _K4(0) }, // 67
  { _K4(K_NUM_ENTER) }, // 68
  { _K4(K_CTRL_R) }, // 69
  { _K4(K_NUM_DIV) }, // 6A
  { _K4(0) }, // 6B
  { _K4(K_ALT_GR) }, // 6C
  { _K4(0) }, // 6D
  { _K4(K_HOME) }, // 6E
  { _K4(K_ARROW_UP) }, // 6F
  { _K4(K_PAGE_UP) }, // 70
  { _K4(K_ARROW_LEFT) }, // 71
  { _K4(K_ARROW_RIGHT) }, // 72
  { _K4(K_END) }, // 73
  { _K4(K_ARROW_DOWN) }, // 74
  { _K4(K_PAGE_DOWN) }, // 75
  { _K4(K_INSERT) }, // 76
  { _K4(K_DELETE) }, // 77
  { _K4(0) }, // 78
  { _K4(0) }, // 79
  { _K4(0) }, // 7A
  { _K4(0) }, // 7B
  { _K4(0) }, // 7C
  { _K4(0) }, // 7D
  { _K4(0) }, // 7E
  { _K4(0) }, // 7F
  { _K4(0) }, // 80
  { _K4(0) }, // 81
  { _K4(0) }, // 82
  { _K4(0) }, // 83
  { _K4(0) }, // 84
  { _K4(K_SYSTEM) }, // 85
  { _K4(0) }, // 86
  { _K4(0) }, // 87
  { _K4(0) }, // 88
  { _K4(0) }, // 89
  { _K4(0) }, // 8A
  { _K4(0) }, // 8B
  { _K4(0) }, // 8C
  { _K4(0) }, // 8D
  { _K4(0) }, // 8E
  { _K4(0) }, // 8F
};

