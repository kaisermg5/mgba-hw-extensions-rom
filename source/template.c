
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>
#include "mgba-extensions.h"

// ROM Data
// strings that wil be copied to the RAM extension
#define STR1_RAM_EXT_INDEX 0xF
#define STR2_RAM_EXT_INDEX 0x3E
const char str1[] = "This is the first text.\n";
const char str2[] = "And this, the second one.\n";


// RAM Data
// just something to get a pointer from
char ramData[100];


void onError(u32 value) {
	iprintf("Error! 0x%lX\n", value);
}

void printSecondString(u32 previousStepReturnCode) {
	if (previousStepReturnCode == HWEX_RET_OK) {
		iprintf(ramData);
		iprintf("\nDone!\n");
	}
}

void printFirstStringLoadSecond(u32 previousStepReturnCode) {
	if (previousStepReturnCode == HWEX_RET_OK) {
		iprintf(ramData);

		MoreRamExtensionRead(
			ramData, 
			sizeof(str2), 	
			STR2_RAM_EXT_INDEX, 
			printSecondString, 	
			onError
		);
	}
}

void loadFirstString(u32 previousStepReturnCode) {
	if (previousStepReturnCode == HWEX_RET_OK) {
		MoreRamExtensionRead(
			ramData, 
			sizeof(str1), 	
			STR1_RAM_EXT_INDEX, 
			printFirstStringLoadSecond, 	
			onError
		);
	}
}

void writeSecondStringToRamExtension(u32 previousStepReturnCode) {
	if (previousStepReturnCode == HWEX_RET_OK) {
		// if there's no need to wait or do anything
		MoreRamExtensionWrite(
			str2, 				// pointer to data
			sizeof(str2), 		// data size
			STR2_RAM_EXT_INDEX, // index to write the data
			loadFirstString, 	
			onError
		);
	} /* else {
		// if there was something to wait or do...
	} */
}

void writeFirstStringToRamExtension(u32 previousStepReturnCode) {
	if (previousStepReturnCode == HWEX_RET_OK) {
		MoreRamExtensionWrite(
			str1, 								// pointer to data
			sizeof(str1), 						// data size
			STR1_RAM_EXT_INDEX, 				// index to write the data
			writeSecondStringToRamExtension,	// success callback: copy the second string 	
			onError								// error callback
		);
	}
}

void initRamExtension(void) {
	MoreRamExtensionInit(
		0x100000,
		writeFirstStringToRamExtension,		// success callback: copy the second string 	
		onError								// error callback
	);
}


#define DELAY 0x1F

//---------------------------------------------------------------------------------
// Entry point
//---------------------------------------------------------------------------------
int main(void) {
	// Declare structure in stack
	struct MgbaExtensionsData extensionsData;

	// Sme devkitARM stuff
	irqInit();
	irqEnable(IRQ_VBLANK);
	consoleDemoInit();

	// Init the extensions library
	InitMgbaExtensions(&extensionsData);

	
	u32 flag = 0;
	u32 index = 0;
	while (1) {
		// Call my code
		if (!flag && extensionsData.active) {
			iprintf("Extensions active\n\n");
			initRamExtension();
			flag = 1;
		}
		
		
		if ((index++ & DELAY) == 0) {
			// Let the extension library do its thing
			HandleMgbaExtensions();
		}

		// VBlank
		VBlankIntrWait();
	}
}

