//
// Copyright (C) 2020~2020 by CSSlayer
// wengxt@gmail.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _FCITX5_LIBTHAI_THAIKB_H_
#define _FCITX5_LIBTHAI_THAIKB_H_

enum class ThaiKBMap { KETMANEE, PATTACHOTE, TIS820_2538 };

unsigned char ThaiKeycodeToChar(ThaiKBMap map, int keycode, int shiftLevel);

#endif // _FCITX5_LIBTHAI_THAIKB_H_
