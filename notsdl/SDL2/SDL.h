#ifndef NOTSDL_H
#define NOTSDL_H

// Keyboard

typedef enum
{
	SDL_SCANCODE_GRAVE = 0,
	SDL_SCANCODE_1 = 1,
	SDL_SCANCODE_2 = 2,
	SDL_SCANCODE_3 = 3,
	SDL_SCANCODE_4 = 4,
	SDL_SCANCODE_5 = 5,
	SDL_SCANCODE_6 = 6,
	SDL_SCANCODE_7 = 7,
	SDL_SCANCODE_8 = 8,
	SDL_SCANCODE_9 = 9,
	SDL_SCANCODE_0 = 10,
	SDL_SCANCODE_MINUS = 11,
	SDL_SCANCODE_EQUALS = 12,
	SDL_SCANCODE_BACKSLASH = 13,
	// 14
	SDL_SCANCODE_KP_0 = 15,
	SDL_SCANCODE_Q = 16,
	SDL_SCANCODE_W = 17,
	SDL_SCANCODE_E = 18,
	SDL_SCANCODE_R = 19,
	SDL_SCANCODE_T = 20,
	SDL_SCANCODE_Y = 21,
	SDL_SCANCODE_U = 22,
	SDL_SCANCODE_I = 23,
	SDL_SCANCODE_O = 24,
	SDL_SCANCODE_P = 25,
	SDL_SCANCODE_LEFTBRACKET = 26,
	SDL_SCANCODE_RIGHTBRACKET = 27,
	// 28
	SDL_SCANCODE_KP_1 = 29,
	SDL_SCANCODE_KP_2 = 30,
	SDL_SCANCODE_KP_3 = 31,
	SDL_SCANCODE_A = 32,
	SDL_SCANCODE_S = 33,
	SDL_SCANCODE_D = 34,
	SDL_SCANCODE_F = 35,
	SDL_SCANCODE_G = 36,
	SDL_SCANCODE_H = 37,
	SDL_SCANCODE_J = 38,
	SDL_SCANCODE_K = 39,
	SDL_SCANCODE_L = 40,
	SDL_SCANCODE_SEMICOLON = 41,
	SDL_SCANCODE_APOSTROPHE = 42,
	// 43 - key next to Enter on ISO
	// 44
	SDL_SCANCODE_KP_4 = 45,
	SDL_SCANCODE_KP_5 = 46,
	SDL_SCANCODE_KP_6 = 47,
	// 48 - ISO LESS-THAN SIGN and GREATER-THAN SIGN
	SDL_SCANCODE_Z = 49,
	SDL_SCANCODE_X = 50,
	SDL_SCANCODE_C = 51,
	SDL_SCANCODE_V = 52,
	SDL_SCANCODE_B = 53,
	SDL_SCANCODE_N = 54,
	SDL_SCANCODE_M = 55,
	SDL_SCANCODE_COMMA = 56,
	SDL_SCANCODE_PERIOD = 57,
	SDL_SCANCODE_SLASH = 58,
	// 59
	SDL_SCANCODE_KP_PERIOD = 60,
	SDL_SCANCODE_KP_7 = 61,
	SDL_SCANCODE_KP_8 = 62,
	SDL_SCANCODE_KP_9 = 63,
	SDL_SCANCODE_SPACE = 64,
	SDL_SCANCODE_BACKSPACE = 65,
	SDL_SCANCODE_TAB = 66,
	SDL_SCANCODE_KP_ENTER = 67,
	SDL_SCANCODE_RETURN = 68,
	SDL_SCANCODE_ESCAPE = 69,
	SDL_SCANCODE_DELETE = 70,
	// 71
	// 72
	// 73
	SDL_SCANCODE_KP_MINUS = 74,
	// 75
	SDL_SCANCODE_UP = 76,
	SDL_SCANCODE_DOWN = 77,
	SDL_SCANCODE_RIGHT = 78,
	SDL_SCANCODE_LEFT = 79,
	SDL_SCANCODE_F1 = 80,
	SDL_SCANCODE_F2 = 81,
	SDL_SCANCODE_F3 = 82,
	SDL_SCANCODE_F4 = 83,
	SDL_SCANCODE_F5 = 84,
	SDL_SCANCODE_F6 = 85,
	SDL_SCANCODE_F7 = 86,
	SDL_SCANCODE_F8 = 87,
	SDL_SCANCODE_F9 = 88,
	SDL_SCANCODE_F10 = 89,
	SDL_SCANCODE_NUMLOCKCLEAR = 90, // Numpad ( or [
	SDL_SCANCODE_SCROLLLOCK = 91, // Numpad ) or ]
	SDL_SCANCODE_KP_DIVIDE = 92,
	SDL_SCANCODE_KP_MULTIPLY = 93,
	SDL_SCANCODE_KP_PLUS = 94,
	SDL_SCANCODE_F12 = 95, // Help key
	SDL_SCANCODE_LSHIFT = 96,
	SDL_SCANCODE_RSHIFT = 97,
	SDL_SCANCODE_CAPSLOCK = 98,
	SDL_SCANCODE_LCTRL = 99,
	SDL_SCANCODE_LALT = 100,
	SDL_SCANCODE_RALT = 101,
	SDL_SCANCODE_LGUI = 102,
	SDL_SCANCODE_RGUI = 103,

	SDL_SCANCODE_UNKNOWN = 127,
	SDL_NUM_SCANCODES = 128
} SDL_Scancode;

#define SDL_SCANCODE_RCTRL SDL_SCANCODE_UNKNOWN
#define SDL_SCANCODE_SYSREQ SDL_SCANCODE_UNKNOWN
#define SDL_SCANCODE_F11 SDL_SCANCODE_UNKNOWN
#define SDL_SCANCODE_PRINTSCREEN SDL_SCANCODE_UNKNOWN
#define SDL_SCANCODE_INSERT SDL_SCANCODE_UNKNOWN
#define SDL_SCANCODE_HOME SDL_SCANCODE_UNKNOWN
#define SDL_SCANCODE_PAGEUP SDL_SCANCODE_UNKNOWN
#define SDL_SCANCODE_END SDL_SCANCODE_UNKNOWN
#define SDL_SCANCODE_PAGEDOWN SDL_SCANCODE_UNKNOWN

// Endianness

#define SDL_LIL_ENDIAN  1234
#define SDL_BIG_ENDIAN  4321
#define SDL_BYTEORDER   SDL_BIG_ENDIAN

#include <stdint.h>

#define SDL_Swap16(x) (uint16_t)(((uint16_t)(x) & 0x00ff) << 8 | ((uint16_t)(x) & 0xff00) >> 8)
#define SDL_Swap32(x) (uint32_t)(((uint32_t)(x) & 0x000000ff) << 24 | ((uint32_t)(x) & 0x0000ff00) <<  8 | \
                                 ((uint32_t)(x) & 0x00ff0000) >>  8 | ((uint32_t)(x) & 0xff000000) >> 24)

static inline float SDL_SwapFloat(float x)
{
	union
	{
		float f;
		uint32_t ui32;
	} swapper;
	swapper.f = x;
	swapper.ui32 = SDL_Swap32(swapper.ui32);
	return swapper.f;
}

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define SDL_SwapLE16(X)     (X)
#define SDL_SwapLE32(X)     (X)
#define SDL_SwapFloatLE(X)  (X)
#define SDL_SwapBE16(X)     SDL_Swap16(X)
#define SDL_SwapBE32(X)     SDL_Swap32(X)
#define SDL_SwapFloatBE(X)  SDL_SwapFloat(X)
#else
#define SDL_SwapLE16(X)     SDL_Swap16(X)
#define SDL_SwapLE32(X)     SDL_Swap32(X)
#define SDL_SwapFloatLE(X)  SDL_SwapFloat(X)
#define SDL_SwapBE16(X)     (X)
#define SDL_SwapBE32(X)     (X)
#define SDL_SwapFloatBE(X)  (X)
#endif

// some types

typedef uint32_t Uint32;
typedef uint16_t Uint16;
typedef uint8_t Uint8;

//#define SDL_INIT_VIDEO 1

// audio
#define AUDIO_S8        0x8008  /**< Signed 8-bit samples */
#define AUDIO_S16LSB    0x8010  /**< Signed 16-bit samples */
#define AUDIO_S16MSB    0x9010  /**< As above, but big-endian byte order */
#define AUDIO_U16       AUDIO_U16LSB
#define AUDIO_S16       AUDIO_S16LSB

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define AUDIO_S16SYS    AUDIO_S16LSB
#else
#define AUDIO_S16SYS    AUDIO_S16MSB
#endif
typedef Uint32 SDL_AudioDeviceID;
typedef Uint16 SDL_AudioFormat;
typedef void (*SDL_AudioCallback)(void *userdata, Uint8 * stream, int len);
typedef struct SDL_AudioSpec
{
    int freq;
    SDL_AudioFormat format;
    Uint8 channels;
    //Uint8 silence;
    Uint16 samples;
    //Uint16 padding;
    //Uint32 size;
    SDL_AudioCallback callback;
    void *userdata;
} SDL_AudioSpec;

SDL_AudioDeviceID SDL_OpenAudioDevice(const char *device, int iscapture, const SDL_AudioSpec *desired, SDL_AudioSpec *obtained, int allowed_changes);
void SDL_CloseAudioDevice(SDL_AudioDeviceID dev);
void SDL_PauseAudioDevice(SDL_AudioDeviceID dev, int pause_on);
const char* SDL_GetError(void);

// dummy functions and wrappers

#define SDL_INIT_AUDIO 0x00000010
#define SDL_INIT_VIDEO 0x00000020

#define SDL_Log printf
#define SDL_memset memset

#define SDL_InitSubSystem(flags) (0)
//#define SDL_GetTicks() (0)

Uint32 SDL_GetTicks(void);
int SDL_Init(Uint32 flags);
void SDL_Quit(void);

#endif

