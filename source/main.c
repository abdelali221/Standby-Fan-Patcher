#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <unistd.h>
#include <string.h>

#include "ios.h"
#include "patcher.h"

#define WHITE_BG_BLACK_FG "\x1b[47;1m\x1b[30m"
#define RED_BG_WHITE_FG "\x1b[101;93m"
#define DEFAULT_BG_FG "\x1b[40;37m"

static void init(void)
{
	void *xfb;
	GXRModeObj *rmode;

	VIDEO_Init();
	WPAD_Init();
	rmode = VIDEO_GetPreferredMode(NULL);
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
}

void POSCursor(uint8_t X, uint8_t Y) {
	printf("\x1b[%d;%dH", Y, X);
}


u8 instr_patch[] = {0xE1, 0xA0, 0x00, 0x00};

u8 STM_hash1[] = {  // 2007, Ver offset : 0x392C
	0xFF, 0xF7, 0xBE, 0x43,
	0x36, 0x43, 0x35, 0xD5,
	0x56, 0x8B, 0x10, 0x75,
	0x90, 0x33, 0x29, 0xB6,
	0x2E, 0xBB, 0xAD, 0xE6 
};

u8 instr_match1[] = {0xEB, 0x00, 0x02, 0x0B};

u8 STM_hash2[] = { // 2008, Ver offset : 0x3A88
	0x49, 0x2E, 0x1A, 0x87,
	0xDE, 0xF9, 0xA6, 0x4B,
	0xBA, 0xAD, 0x80, 0x3C,
	0xD9, 0x58, 0xD3, 0xA2,
	0x78, 0xAF, 0x3C, 0xEA
};

u8 instr_match2[] = {0xEB, 0x00, 0x02, 0x07};

u8 STM_hash3[] = { // 2009, Ver offset : 0x398C
	0xFF, 0x38, 0x0D, 0x01,
	0x88, 0xFD, 0x07, 0x30,
	0xBA, 0xF8, 0x37, 0xD4,
	0x78, 0x3E, 0xA6, 0xA1,
	0x8C, 0x84, 0x7B, 0x27
};

u8 instr_match3[] = {0xEB, 0x00, 0x02, 0x1C};

int main(int argc, char **argv) {
	init();
	file_patch_descriptor STM_Patch[3];
	memcpy(STM_Patch[0].hash, STM_hash1, 20);
	STM_Patch[0].ver_str_offset = 0x392C;
	STM_Patch[0].patch_offset = 0xC7C;
	STM_Patch[0].patch_size = 0x4;
	STM_Patch[0].patch = instr_patch;
	STM_Patch[0].match_size = 0x4;
	STM_Patch[0].match = instr_match1;

	memcpy(STM_Patch[1].hash, STM_hash2, 20);
	STM_Patch[1].ver_str_offset = 0x3A88;
	STM_Patch[1].patch_offset = 0xC7C;
	STM_Patch[1].patch_size = 0x4;
	STM_Patch[1].patch = instr_patch;
	STM_Patch[1].match_size = 0x4;
	STM_Patch[1].match = instr_match2;

	memcpy(STM_Patch[2].hash, STM_hash3, 20);
	STM_Patch[2].ver_str_offset = 0x398C;
	STM_Patch[2].patch_offset = 0xC8C;
	STM_Patch[2].patch_size = 0x4;
	STM_Patch[2].patch = instr_patch;
	STM_Patch[2].match_size = 0x4;
	STM_Patch[2].match = instr_match3;
	
	printf("\nStandby Fan Patcher\n Created by Abdelali221");
	printf("\n\nAcquiring Permissions...");
	if(disable_ahbprot()) {
		apply_runtime_ios_patches();
		printf("Done.\n");
	} else {
		printf("Failed!");
		exit(0);
	}
	
	ISFS_Initialize();

	s32 map_fd = ISFS_Open("/shared1/content.map", ISFS_OPEN_READ);
	if(map_fd < 0) {
		printf("Failed to open file! ret: %d", map_fd);
		return 0;
	}

	fstats stats __attribute__((aligned(32))) = {0};
	ISFS_GetFileStats(map_fd, &stats);

	u8 *map_buff = malloc(stats.file_length);
	ISFS_Read(map_fd, map_buff, stats.file_length);

	if(stats.file_length % sizeof(content_map_entry) || !stats.file_length) {
		printf("\nMap File is invalid! size: 0x%X", stats.file_length);
	} else {
		printf("\nMap File size: 0x%X", stats.file_length);
	}

	ISFS_Close(map_fd);

	POSCursor(24, 10);
    printf("This software modifies your");
    POSCursor(35, 11);
    printf("%sNAND!%s", RED_BG_WHITE_FG, DEFAULT_BG_FG);
    POSCursor(26, 13);
    printf("Use it %sAT YOUR OWN RISK!%s", RED_BG_WHITE_FG, DEFAULT_BG_FG);
	POSCursor(15, 15);
	printf("Press A to patch, B to revert or HOME to exit");
	
	while(1) {
		WPAD_ScanPads();
		u32 pressed = WPAD_ButtonsDown(0);

		if ( pressed & WPAD_BUTTON_HOME )
			goto exit;

		else if(pressed & WPAD_BUTTON_A) {
			printf("\x1b[2J");
			for(int i = 0; 
				i < (stats.file_length); 
				i += sizeof(content_map_entry))
			{
				for(u8 j = 0; j < 3; j++) {
					if(!memcmp(STM_Patch[j].hash, &map_buff[i + 8], 20)) {
						char str[22] = {0};
						char name[9] = {0};
						memcpy(name, &map_buff[i], 8);
						sprintf(str, "/shared1/%s.app", name); 
						patch_file(&STM_Patch[j], str);
					}
				}
			}
			printf("\n\nPlease make sure to reinstall the patch if you update any of the IOSes.");
			break;
		} else if(pressed & WPAD_BUTTON_B) {
			printf("\x1b[2J");
			for(int i = 0; 
				i < (stats.file_length); 
				i += sizeof(content_map_entry))
			{
				for(u8 j = 0; j < 3; j++) {
					if(!memcmp(STM_Patch[j].hash, &map_buff[i + 8], 20)) {
						char str[22] = {0};
						char name[9] = {0};
						memcpy(name, &map_buff[i], 8);
						sprintf(str, "/shared1/%s.app", name); 
						revert_file(&STM_Patch[j], str);
					}
				}
			}
			break;
		}

		VIDEO_WaitVSync();
	}

	printf("\n\nPress HOME to exit");

	while(1) {
		WPAD_ScanPads();
		u32 pressed = WPAD_ButtonsDown(0);

		if ( pressed & WPAD_BUTTON_HOME )
			break;
	}
exit:
	printf("\x1b[2J\nExiting...");
	return 0;
}