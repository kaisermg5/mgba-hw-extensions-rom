
#include "mgba-extensions.h"

#define NULL ((void *) 0)

// #define REG_MGBA_EXTENSIONS_ENABLED ((volatile unsigned short *) 0x04FFFA00)
// #define REG_MGBA_EXTENSIONS_CNT ((volatile unsigned short *) 0x04FFFA02)
// #define REG_MGBA_EXTENSION_0 ((volatile u32 *) 0x04FFFA08)
#define ENABLED_VALUE 0x1DEA
#define ENABLE_VALUE 0xC0DE

#define REG_HWEX_ENABLE ((volatile unsigned short *) 0x04400A00)
#define REG_HWEX_VERSION ((volatile unsigned short *) 0x04400A02)
#define REG_HWEX_ENABLE_FLAGS_0 ((volatile unsigned short *) 0x04400A04)
#define REG_HWEX_ENABLE_FLAGS_1 ((volatile unsigned short *) 0x04400A06)
#define REG_HWEX_ENABLE_FLAGS_2 ((volatile unsigned short *) 0x04400A08)
#define REG_HWEX_ENABLE_FLAGS_3 ((volatile unsigned short *) 0x04400A0A)
#define REG_HWEX_ENABLE_FLAGS_4 ((volatile unsigned short *) 0x04400A0C)
#define REG_HWEX_ENABLE_FLAGS_5 ((volatile unsigned short *) 0x04400A0E)
#define REG_HWEX0_CNT ((volatile unsigned short *) 0x04400A10)
#define REG_HWEX0_RET_CODE ((volatile unsigned short *) 0x04400A12)
#define REG_HWEX0_P0 ((volatile unsigned long *) 0x04400A14)
#define REG_HWEX0_P1 ((volatile unsigned long *) 0x04400A18)
#define REG_HWEX0_P2 ((volatile unsigned long *) 0x04400A1C)
#define REG_HWEX0_P3 ((volatile unsigned long *) 0x04400A20)

const volatile unsigned short * extensionCNTs[] = {
    REG_HWEX0_CNT // more ram
};

const volatile unsigned short * extensionReturnCodes[] = {
    REG_HWEX0_RET_CODE // more ram
};

struct MgbaExtensionsData* mgbaExtensionsData;



static void UpdateActiveBit(void) {
    mgbaExtensionsData->active = *REG_HWEX_ENABLE == ENABLED_VALUE;
}


void HandleMgbaExtensions(void) {
    u32 previousActive = mgbaExtensionsData->active;
    UpdateActiveBit();
    if (previousActive != mgbaExtensionsData->active) {
        // TODO: do something
    }
    
    if (mgbaExtensionsData->active) {
        //u32 flags = *REG_MGBA_EXTENSIONS_CNT;
        for (u32 i = 0; i < NUM_OF_EXTENSIONS; i++) {
            if (mgbaExtensionsData->extensionsSuccessCallbacks[i] != NULL
                /*&& (flags & (1 << i))*/) 
            {
                unsigned short cnt = *(extensionCNTs[i]);
                if ((cnt & 0x8000) == 0) {
                    u32 returnCode = *(extensionReturnCodes[i]);
                    MgbaExtensionCallback callback;
                    if (returnCode >= HWEX_RET_ERR_UNKNOWN) {
                        callback = mgbaExtensionsData->extensionsErrorCallbacks[i];
                        mgbaExtensionsData->extensionsErrorCallbacks[i] = NULL;
                        mgbaExtensionsData->extensionsSuccessCallbacks[i] = NULL;
                    } else {
                        callback = mgbaExtensionsData->extensionsSuccessCallbacks[i];
                        if (returnCode == HWEX_RET_OK) {
                            mgbaExtensionsData->extensionsErrorCallbacks[i] = NULL;
                            mgbaExtensionsData->extensionsSuccessCallbacks[i] = NULL;
                        }
                    }

                    if (callback != NULL) {
                        callback(returnCode);
                    }
                }
            }
        }
    } 
}

void InitMgbaExtensions(struct MgbaExtensionsData* data) {
    mgbaExtensionsData = data;
    *REG_HWEX_ENABLE = ENABLE_VALUE;
    UpdateActiveBit();
    for (u32 i = 0; i < NUM_OF_EXTENSIONS; i++) {
        mgbaExtensionsData->extensionsSuccessCallbacks[i] = NULL;
        mgbaExtensionsData->extensionsErrorCallbacks[i] = NULL;
    }

    if (data->active) {
        *REG_HWEX_ENABLE_FLAGS_0 = 1;
    }
}

static u32 SetExtensionHandler(u32 extension, MgbaExtensionCallback successCallback,  MgbaExtensionCallback errorCallback) {
    if (extension < NUM_OF_EXTENSIONS && successCallback && !mgbaExtensionsData->extensionsSuccessCallbacks[extension]) {
        mgbaExtensionsData->extensionsSuccessCallbacks[extension] = successCallback;
        mgbaExtensionsData->extensionsErrorCallbacks[extension] = errorCallback;
        return 1;
    }
    return 0;
}


enum {
    EXTENSION_MORE_RAM = 0
};

// Extension: more RAM
enum {
    MORE_RAM_WRITE = 0,
    MORE_RAM_READ = 1,
    MORE_RAM_SWAP = 2,
};

static u32 MoreRamExtensionBase(u32 action, u32 * addr, u32 size, u32 index, 
    MgbaExtensionCallback successCallback,  MgbaExtensionCallback errorCallback) 
{
    u32 ok = action >= MORE_RAM_WRITE && action <= MORE_RAM_SWAP && addr && size;
    if (ok) {
        if (SetExtensionHandler(EXTENSION_MORE_RAM, successCallback,  errorCallback)) {
            *REG_HWEX0_P0 = action;
            *REG_HWEX0_P1 = index;
            *REG_HWEX0_P2 = (u32) addr;
            *REG_HWEX0_P3 = size;
            *REG_HWEX0_CNT = 1;
        }
        
    }
    return ok;
}


u32 MoreRamExtensionWrite(const void * addr, u32 size, u32 index, 
    MgbaExtensionCallback successCallback,  MgbaExtensionCallback errorCallback) 
{
    return MoreRamExtensionBase(MORE_RAM_WRITE, (u32 *)addr, size, index, successCallback, errorCallback) ;
}

u32 MoreRamExtensionRead(void * addr, u32 size, u32 index, 
    MgbaExtensionCallback successCallback,  MgbaExtensionCallback errorCallback) 
{
    return MoreRamExtensionBase(MORE_RAM_READ, addr, size, index, successCallback, errorCallback) ;
}

u32 MoreRamExtensionSwap(void * addr, u32 size, u32 index, 
    MgbaExtensionCallback successCallback,  MgbaExtensionCallback errorCallback) 
{
    return MoreRamExtensionBase(MORE_RAM_SWAP, addr, size, index, successCallback, errorCallback) ;
}