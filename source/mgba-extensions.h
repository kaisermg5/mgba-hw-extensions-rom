
#ifndef MGBA_EXTENSIONS_H
#define MGBA_EXTENSIONS_H



#define NUM_OF_EXTENSIONS 1

typedef unsigned long u32;
typedef void (*MgbaExtensionCallback)(u32 code);


struct MgbaExtensionsData {
    u32 active: 1;
    u32 _unused: 31;

    MgbaExtensionCallback extensionsSuccessCallbacks[NUM_OF_EXTENSIONS];
    MgbaExtensionCallback extensionsErrorCallbacks[NUM_OF_EXTENSIONS];
};



enum HWEX_RETURN_CODES {
    HWEX_RET_OK = 0,
    HWEX_RET_WAIT = 1,

    // Errors
    HWEX_RET_ERR_UNKNOWN = 0x100,
    HWEX_RET_ERR_BAD_ADDRESS = 0x101,
    HWEX_RET_ERR_INVALID_PARAMETERS = 0x102,
    HWEX_RET_ERR_WRITE_TO_ROM = 0x103
};



void HandleMgbaExtensions(void);
void InitMgbaExtensions(struct MgbaExtensionsData* data);

u32 MoreRamExtensionWrite(const void * addr, u32 size, u32 index, 
    MgbaExtensionCallback successCallback,  MgbaExtensionCallback errorCallback);

u32 MoreRamExtensionRead(void * addr, u32 size, u32 index, 
    MgbaExtensionCallback successCallback,  MgbaExtensionCallback errorCallback);
u32 MoreRamExtensionSwap(void * addr, u32 size, u32 index, 
    MgbaExtensionCallback successCallback,  MgbaExtensionCallback errorCallback);

#endif
