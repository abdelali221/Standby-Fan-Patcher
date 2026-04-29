/*
   Automated IOS Module Patcher

   Copyright (C) 2026 Abdelali221

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _PATCHER_H_
#define _PATCHER_H_


typedef struct content_map_entry {
    char sharedId[8];
    u8 sha1hash[20];
} content_map_entry;

typedef struct file_patch_descriptor {
	u32 ver_str_offset;
    u8 hash[20];
	u32 patch_offset;
	u32 patch_size;
	u8* patch;
	u32 match_size;
	u8* match;
} file_patch_descriptor;

int patch_file(file_patch_descriptor *desc, const char* path);
int revert_file(file_patch_descriptor *desc, const char* path);

#endif