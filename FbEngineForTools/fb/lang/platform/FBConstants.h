#pragma once

// ------------------------------------------------------------------------------
// some preprocessor constants

// never assume anything about these numbers (such as "false" being zero)

// consider these as possible bit masks
// if possible, keep the bits totally independent of each other
// (this is not quite the case currently)

// fb constants: (0x01......)

// builds: (0x011.....)

#define FB_DEBUG         0x01120000
#define FB_RELEASE       0x01140000
#define FB_FINAL_RELEASE 0x01180000

// generic on/off flags: (0x012.....)

#define FB_TRUE     0x01210000
#define FB_FALSE    0x01220000

// platform types: (0x01002...)

#define FB_PC      0x01002100
#define FB_CONSOLE 0x01002200
#define FB_MOBILE  0x01002300

// platforms: (PC 0x010021..) (CONSOLE 0x010022..) (MOBILE 0x010023..)

#define FB_WINDOWS 0x01002110

// compilers: (0x01004...)

#define FB_MSC      0x01004001
#define FB_GNUC     0x01004002
#define FB_ICC      0x01004004
//#define FB_SNC      0x01004008
//#define FB_GHS      0x01004016
#define FB_CLANG    0x01004032

// engines: (0x0101....)
#define FB_PHYSX         0x01010001
#define FB_NULL          0x01010002

// byte orders: (0x01008...)

#define FB_LITTLE_ENDIAN 0x01008004
#define FB_BIG_ENDIAN    0x01008008

// byte orders: (0x01016...)

#define FB_NULL_AUDIO    0x01016001
#define FB_WWISE         0x01016002
#define FB_SOLOUD        0x01016004

// renderer mode: (0x01020....)
#define FB_RENDERER_MODE_NORMAL           0x01020001
#define FB_RENDERER_MODE_GALACTIC         0x01020002


// ------------------------------------------------------------------------------
