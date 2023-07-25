#ifndef PS3M_H
#define PS3M_H

#include <SDI_compiler.h>
#include <devices/ahi.h>

ULONG ASM ps3mInitialize(REG(d0, ULONG modulelength), REG(d1, ULONG soundid), REG(d2, WORD firstchannel),
	REG(a0, APTR moduleaddr), REG(a1, struct Library *ahibase), REG(a2, struct AHIAudioCtrl *actrl),
	REG(a3, HOOKFUNC *playerfunc), REG(a4, HOOKFUNC *soundfunc));

void ASM ps3mStop(void);

void ASM ps3mContinue(void);

void ASM ps3mUninitialize(void);

ULONG ASM ps3mIsPlaying(void);

ULONG ASM ps3mGetSongPosition(void);

ULONG ASM ps3mGetRowPosition(void);

ULONG ASM ps3mSetSongAndRowPosition(REG(d0, ULONG order), REG(d1, ULONG row));

void ASM ps3mSetVolume(REG(d0, UWORD volume));

#endif
