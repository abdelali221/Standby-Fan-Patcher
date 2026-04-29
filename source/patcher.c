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

#include <gccore.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "patcher.h"

int patch_file(file_patch_descriptor *desc, const char* path) {
	if(!path) return -1;
	s32 fd = ISFS_Open(path, ISFS_OPEN_RW);
	if(fd < 0) {
		printf("\nFailed to open file %s!", path);
		return -2;
	}
	fstats stats __attribute__((aligned(32))) = {0};
	ISFS_GetFileStats(fd, &stats);
	printf("\nPatching %s at 0x%08X | size: 0x%X", path, desc->patch_offset, stats.file_length);
	
	ISFS_Seek(fd, desc->patch_offset, SEEK_SET);
	u8 match[desc->match_size] __attribute__((aligned(32)));
	ISFS_Read(fd, match, desc->match_size);
	if(memcmp(match, desc->match, desc->match_size)) {
		printf("\n Expected values did not match!");
		uint32_t found = 0;
		memcpy(&found, match, 4);
		uint32_t expected = 0;
		memcpy(&expected, desc->match, 4);
		printf(" Found: 0x%08X Expected: 0x%08X", found, expected);
		ISFS_Close(fd);
		return -3;
	}
	ISFS_Seek(fd, desc->patch_offset, SEEK_SET);
	ISFS_Write(fd, desc->patch, desc->patch_size);
	ISFS_Close(fd);
	return 0;
}

int revert_file(file_patch_descriptor *desc, const char* path) {
	if(!path) return -1;
	s32 fd = ISFS_Open(path, ISFS_OPEN_RW);
	if(fd < 0) return -1;
	printf("\nReverting %s at 0x%08X", path, desc->patch_offset);
	
	ISFS_Seek(fd, desc->patch_offset, SEEK_SET);
	u8 match[desc->patch_size] __attribute__((aligned(32)));
	ISFS_Read(fd, match, desc->patch_size);
	if(memcmp(match, desc->patch, desc->patch_size)) {
		printf("\n Expected values did not match!");
		uint32_t found = 0;
		memcpy(&found, match, 4);
		uint32_t expected = 0;
		memcpy(&expected, desc->patch, 4);
		printf(" Found: 0x%08X Expected: 0x%08X", found, expected);
		ISFS_Close(fd);
		return -2;
	}
	ISFS_Seek(fd, desc->patch_offset, SEEK_SET);
	ISFS_Write(fd, desc->match, desc->match_size);
	ISFS_Close(fd);
	return 0;
}