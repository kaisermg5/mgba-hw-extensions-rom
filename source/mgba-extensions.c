
#include "mgba-extensions.h"

#define NULL ((void *) 0)

#define ENABLED_VALUE 0x1DEA
#define ENABLE_CODE 0xC0DE

#define REG_HWEX_ENABLE ((volatile unsigned short *) 0x04400A00)
#define REG_HWEX_VERSION ((volatile unsigned short *) 0x04400A02)
#define REG_HWEX0_ENABLE ((volatile unsigned short *) 0x04400A04)
#define REG_HWEX0_CNT ((volatile unsigned short *) 0x04400A06)
#define REG_HWEX0_RET_CODE ((volatile unsigned short *) 0x04400A08)
#define REG_HWEX0_UNUSED ((volatile unsigned short *) 0x04400A0A)
#define REG_HWEX0_P0 ((volatile unsigned long *) 0x04400A0C)
#define REG_HWEX0_P1 ((volatile unsigned long *) 0x04400A10)
#define REG_HWEX0_P2 ((volatile unsigned long *) 0x04400A14)
#define REG_HWEX0_P3 ((volatile unsigned long *) 0x04400A18)

const volatile unsigned short * extensionEnableRegisters[] = {
    REG_HWEX0_ENABLE // more ram
};

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
        for (u32 i = 0; i < NUM_OF_EXTENSIONS; i++) {
            if (mgbaExtensionsData->extensionsSuccessCallbacks[i] != NULL
                && *extensionEnableRegisters[i] == ENABLED_VALUE) 
            {
                unsigned short cnt = *(extensionCNTs[i]);
                if ((cnt & 0x8000) == 0) {
                    u32 returnCode = *extensionReturnCodes[i];
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
    *REG_HWEX_ENABLE = ENABLE_CODE;
    UpdateActiveBit();
    for (u32 i = 0; i < NUM_OF_EXTENSIONS; i++) {
        mgbaExtensionsData->extensionsSuccessCallbacks[i] = NULL;
        mgbaExtensionsData->extensionsErrorCallbacks[i] = NULL;
    }

    if (data->active) {
        *REG_HWEX0_ENABLE = ENABLE_CODE;
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
    MORE_RAM_INIT = 3,
    MORE_RAM_RESIZE = 4,
    MORE_RAM_DESTROY = 5,
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

u32 MoreRamExtensionInit(u32 size, MgbaExtensionCallback successCallback,  MgbaExtensionCallback errorCallback) 
{
    if (SetExtensionHandler(EXTENSION_MORE_RAM, successCallback,  errorCallback)) {
        *REG_HWEX0_P0 = MORE_RAM_INIT;
        *REG_HWEX0_P1 = size;
        *REG_HWEX0_CNT = 1;
        return 1;
    }
    return 0;
}