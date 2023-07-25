#include "drally.h"
#include "draudio.h"
#include "drmemory.h"
#include "drencryption.h"
#include "bpa.h"

//#define DOOMSOUND
#define PS3M
//#define AHI_DEBUG

#include <proto/dos.h>
#include <proto/exec.h>
#ifdef DOOMSOUND
#include <proto/doomsound.h>
#include <exec/execbase.h>
#else
#include <proto/utility.h>
#include <proto/ahi.h>
#include <devices/ahi.h>
#ifdef PS3M
#include <ps3m.h>
#endif
#endif
#include <clib/debug_protos.h>

#include <SDI_compiler.h>
#include <SDI_hook.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// AHIST_S8S sample descriptor
typedef struct sount_s {
	__POINTER__ sample;
	//__DWORD__ volume;
	__DWORD__ frequency;
	__WORD__ length;
	__WORD__ loop;
	//__WORD__ padding;
} sound_t;

static int *soundMap; // instrument to sound
static int soundCount;
static sound_t *sounds;
static int soundChannels;
#ifdef PS3M
static int musicChannels;
static void *musicData;
static int musicSize;
#endif

static __DWORD__ msx_v, sfx_v;

__DWORD__ SOUND_SAMPLES = 2048;
__BYTE__ SOUND_LOADED = 0;
__BYTE__ SOUND_PLAYING = 0;
__BYTE__ SOUND = 0;
__WORD__ SOUND_SAMPLERATE = 0x5622;
__DWORD__ MASTER_VOLUME = 0x10000;
__DWORD__ MSX_VOLUME = 0x10000;
__DWORD__ SFX_VOLUME = 0x10000;
__WORD__ ___24e750h[32]; // which instrument is playing on the channel


void dRally_System_addExitCallback(void_cb);
void dRally_System_removeExitCallback(void_cb);
__DWORD__ MULSHIFT(__DWORD__, __DWORD__);
void ___68c42h_cdecl(void);


#ifdef DOOMSOUND
struct Library *DoomSndBase = NULL;
#else
struct Library *AHIBase;
static struct MsgPort *AHImp = NULL;
static struct AHIRequest *AHIio = NULL;
static BYTE AHIDevice = -1;
static struct AHIAudioCtrl *actrl = NULL;
static ULONG audioID = AHI_DEFAULT_ID;
#ifdef PS3M
HOOKPROTO(hookWrapper, ULONG, struct AHIAudioCtrl *actrl, struct AHISoundMessage *smsg)
{
	// jump to h_SubEntry if it's not NULL
	__asm__ volatile (
		"tst.l  12(a0)" "\n\t"
		"jeq    1f" "\n\t"
		"move.l 12(a0),a0" "\n\t"
		"jsr    (a0)" "\n\t"
		"1:"
	);
	return 0;
}
MakeHook(SoundHook, hookWrapper);
MakeHook(PlayerHook, hookWrapper);

HOOKPROTO(filterFunc, ULONG, struct AHIAudioModeRequester *req, ULONG modeid)
{
	ULONG stereo, panning;
	//char modename[64];
	struct TagItem queryTags[] =
	{
		{AHIDB_Stereo, (IPTR)&stereo},
		{AHIDB_Panning, (IPTR)&panning},
		//{AHIDB_BufferLen, sizeof(modename)},
		//{AHIDB_Name, (IPTR)modename},
		{TAG_DONE, 0}
	};
	AHI_GetAudioAttrsA(modeid, NULL, queryTags);
	//printf("mode %08x - %s stereo %d panning %d ret %d\n", modeid, modename, stereo, panning, (!stereo || panning));
	// stereo modes must support panning
	return (!stereo || panning);
}
MakeHook(FilterHook, filterFunc);
#endif
#ifdef AHI_DEBUG
#include <exec/libraries.h>
#include <SDI/SDI_lib.h>

#define LIBCALL_4(ret,func,callbase,r1,n1,r2,n2,r3,n3,r4,n4) \
	((ret ASM (*) (REG(r1, ULONG), REG(r2, ULONG), REG(r3, ULONG), REG(r4, ULONG), REG(a6, APTR)))func) \
	((ULONG)n1, (ULONG)n2, (ULONG)n3, (ULONG)n4, (APTR)callbase)

#define LIBCALL_5(ret,func,callbase,r1,n1,r2,n2,r3,n3,r4,n4,r5,n5) \
	((ret ASM (*) (REG(r1, ULONG), REG(r2, ULONG), REG(r3, ULONG), REG(r4, ULONG), REG(r5, ULONG), REG(a6, APTR)))func) \
	((ULONG)n1, (ULONG)n2, (ULONG)n3, (ULONG)n4, (ULONG)n5, (APTR)callbase)

#define LIBCALL_6(ret,func,callbase,r1,n1,r2,n2,r3,n3,r4,n4,r5,n5,r6,n6) \
	((ret ASM (*) (REG(r1, ULONG), REG(r2, ULONG), REG(r3, ULONG), REG(r4, ULONG), REG(r5, ULONG), REG(r6, ULONG), REG(a6, APTR)))func) \
	((ULONG)n1, (ULONG)n2, (ULONG)n3, (ULONG)n4, (ULONG)n5, (ULONG)n6, (APTR)callbase)

static APTR oldSetFreq;
static APTR oldSetSound;
static APTR oldSetVol;


LIBPROTO(mySetFreq, void, REG(d0, UWORD channel), REG(d1, ULONG freq), REG(a2, struct AHIAudioCtrl *audioctrl), REG(d2, ULONG flags), REG(a6, struct Library *AHIBase))
{
	//kprintf("AHI_SetFreq  channel %ld\n", channel);
	LIBCALL_4(void, oldSetFreq, AHIBase, d0, channel, d1, freq, a2, audioctrl, d2, flags);
}
LIBPROTO(mySetSound, void, REG(d0, UWORD channel), REG(d1, UWORD sound), REG(d2, ULONG offset), REG(d3, LONG length), REG(a2, struct AHIAudioCtrl *audioctrl), REG(d4, ULONG flags), REG(a6, struct Library *AHIBase))
{
	kprintf("AHI_SetSound channel %ld\n", channel);
	LIBCALL_6(void, oldSetSound, AHIBase, d0, channel, d1, sound, d2, offset, d3, length, a2, audioctrl, d4, flags);
}
LIBPROTO(mySetVol, void, REG(d0, UWORD channel), REG(d1, Fixed volume), REG(d2, sposition pan), REG(a2, struct AHIAudioCtrl *audioctrl), REG(d3, ULONG flags), REG(a6, struct Library *AHIBase))
{
	//kprintf("AHI_SetVol   channel %ld\n", channel);
	LIBCALL_5(void, oldSetVol, AHIBase, d0, channel, d1, volume, d2, pan, a2, audioctrl, d3, flags);
}

static void InstallAHIPatch(void)
{
	Forbid();
	oldSetVol = SetFunction(AHIBase, -11 * LIB_VECTSIZE, (APTR)LFUNC(mySetVol));
	oldSetFreq = SetFunction(AHIBase, -12 * LIB_VECTSIZE, (APTR)LFUNC(mySetFreq));
	oldSetSound = SetFunction(AHIBase, -13 * LIB_VECTSIZE, (APTR)LFUNC(mySetSound));
	Permit();
}

static void RemoveAHIPatch(void)
{
	Forbid();
	SetFunction(AHIBase, -11 * LIB_VECTSIZE, oldSetVol);
	SetFunction(AHIBase, -12 * LIB_VECTSIZE, oldSetFreq);
	SetFunction(AHIBase, -13 * LIB_VECTSIZE, oldSetSound);
	Permit();
}
#endif
#endif




void dRally_Sound_init(__BYTE__ sound){

	if((SOUND = !!sound)){

#ifdef DOOMSOUND
		Forbid();
		if ((DoomSndBase = (struct Library *)FindName(&SysBase->LibList, "doomsound.library"))) {
			// try to expunge older versions
			if (DoomSndBase->lib_Version < 38) {
				RemLibrary(DoomSndBase);
			}
		}
		Permit();

		if ((DoomSndBase = OpenLibrary("doomsound.library", 38))) {
			Sfx_SetVol(255);
			return;
		}
#else
		if(!AHIBase) {
			if ((AHImp = CreateMsgPort())) {
				if ((AHIio = (struct AHIRequest *)CreateIORequest(AHImp, sizeof(struct AHIRequest)))) {
					AHIio->ahir_Version = 4;
					if (!(AHIDevice = OpenDevice((STRPTR)AHINAME, AHI_NO_UNIT, (struct IORequest *)AHIio, 0))) {
						//ULONG type;
						struct TagItem *item;

						AHIBase = (struct Library *)AHIio->ahir_Std.io_Device;
#ifdef AHI_DEBUG
						InstallAHIPatch();
#endif

						// query the Music Unit mode
						ULONG realtime, stereo, volume, panning, bits, channels;
						struct TagItem queryTags[] =
						{
							{AHIDB_AudioID, (IPTR)&audioID},
							{AHIDB_Realtime, (IPTR)&realtime},
							{AHIDB_Stereo, (IPTR)&stereo},
							{AHIDB_Volume, (IPTR)&volume},
							{AHIDB_Panning, (IPTR)&panning},
							{AHIDB_Bits, (IPTR)&bits},
							{AHIDB_MaxChannels, (IPTR)&channels},
							{TAG_DONE, 0}
						};
						AHI_GetAudioAttrsA(AHI_DEFAULT_ID, NULL, queryTags);
						//printf("%s default id %08x\n", __FUNCTION__, audioID);
						struct TagItem filterTags[] =
						{
							{AHIDB_AudioID, audioID},
							{AHIDB_Realtime, TRUE},
							{AHIDB_Stereo, TRUE},
							{AHIDB_Volume, TRUE},
							{AHIDB_Panning, TRUE},
							{AHIDB_HiFi, FALSE},
							{AHIDB_Bits, 8},
							{AHIDB_MaxChannels, 32},
							{AHIB_Dizzy, (IPTR)&filterTags[1]}, // skips AHIDB_AudioID
							{TAG_DONE, 0}
						};

						if (!realtime || (stereo && !panning) || !volume || bits < 8 || channels < 32) {
							audioID = AHI_BestAudioIDA(filterTags);
							//printf("%s best id %08x\n", __FUNCTION__, audioID);
						} else {
							//audioID = AHI_DEFAULT_ID;
							// find out the default mixing freq through a dummy audio control
							struct AHIAudioCtrl *actrl = AHI_AllocAudio(AHIA_Sounds, 1, AHIA_Channels, 1, TAG_DONE);
							if (actrl) {
								ULONG mixfreq;
								AHI_ControlAudio(actrl,
									AHIC_MixFreq_Query, (IPTR)&mixfreq,
									TAG_END);
								AHI_FreeAudio(actrl);
								SOUND_SAMPLERATE = mixfreq;
								//printf("%s default sample rate %d\n", __FUNCTION__, SOUND_SAMPLERATE);
							}
						}

						extern int _argc;
						if (_argc == 0) {
							Tag filterArray[] = {AHIDB_AudioID, AHIDB_Stereo, AHIDB_Panning, AHIDB_HiFi, TAG_DONE};
							FilterTagItems(filterTags, filterArray, TAGFILTER_NOT);
							struct AHIAudioModeRequester *req = AHI_AllocAudioRequest(
								AHIR_TitleText, (IPTR)"Select a mode and rate",
								AHIR_InitialMixFreq, SOUND_SAMPLERATE,
								AHIR_DoMixFreq, TRUE,
								AHIR_InitialAudioID, audioID,
								AHIR_FilterTags, (IPTR)filterTags,
								AHIR_FilterFunc, (IPTR)&FilterHook,
								TAG_DONE);

							if (req) {
								if (AHI_AudioRequestA(req, NULL)) {
									audioID = req->ahiam_AudioID;
									//printf("%s requested id %08x\n", __FUNCTION__, audioID);
									SOUND_SAMPLERATE = req->ahiam_MixFreq;
									//printf("%s requested sample rate %d\n", __FUNCTION__, SOUND_SAMPLERATE);
								}
								AHI_FreeAudioRequest(req);
							}
						}
						char modename[64];
						AHI_GetAudioAttrs(audioID, NULL,
									AHIDB_BufferLen, sizeof(modename),
									AHIDB_Name, (IPTR)modename,
									TAG_END);
						printf("[dRally.SOUND] AHI mode: %s\n", modename);
						printf("[dRally.SOUND] sample rate: %d\n", SOUND_SAMPLERATE);
						return;
					}
					DeleteIORequest((struct IORequest *)AHIio);
					AHIio = NULL;
				}
				DeleteMsgPort(AHImp);
				AHImp = NULL;
			}
		}
#endif
		SOUND = 0;
	}
}

void dRally_Sound_quit(void){

#ifdef DOOMSOUND
	if (DoomSndBase)
	{
		CloseLibrary(DoomSndBase);
		DoomSndBase = NULL;
	}
#else
	if (!AHIDevice) {
		printf("[dRally.SOUND] Closing AHI device.\n");
#ifdef AHI_DEBUG
		RemoveAHIPatch();
#endif
		CloseDevice((struct IORequest *)AHIio);
		AHIDevice = -1;
		AHIBase = NULL;
	}

	if (AHIio) {
		DeleteIORequest((struct IORequest *)AHIio);
		AHIio = NULL;
	}

	if (AHImp) {
		DeleteMsgPort(AHImp);
		AHImp = NULL;
	}
#endif
}

void ___65788h_updateVolume_cdecl(void)
{
	msx_v = MULSHIFT(MASTER_VOLUME, MSX_VOLUME);
	sfx_v = MULSHIFT(MASTER_VOLUME, SFX_VOLUME);
#ifdef PS3M
	ps3mSetVolume(msx_v / 1024);
#endif
}

// 00065710h
void dRally_Sound_setMasterVolume(__DWORD__ vol){

	if(SOUND&&SOUND_LOADED){
	
		MASTER_VOLUME = vol;
		___65788h_updateVolume_cdecl();
	}
}

// 0006572ch
void dRally_Sound_setMusicVolume(__DWORD__ vol){

	if(SOUND&&SOUND_LOADED){
	
		MSX_VOLUME = vol;
		___65788h_updateVolume_cdecl();
	}
}

// 00065770h
void dRally_Sound_setEffectsVolume(__DWORD__ vol){

	if(SOUND&&SOUND_LOADED){

		SFX_VOLUME = vol;
		___65788h_updateVolume_cdecl();
	}
}

// 000649a8h
void dRally_Sound_stop(void){

	if(SOUND&&SOUND_LOADED&&SOUND_PLAYING){

#ifdef DOOMSOUND
		// nothing to do here
#else
		AHI_ControlAudio(actrl, AHIC_Play, FALSE, TAG_END);
#endif

		SOUND_PLAYING = 0;
	}

	dRally_System_removeExitCallback(dRally_Sound_stop);
}

// 00064a28h
void dRally_Sound_release(void){

	dRally_Sound_stop();

	if(SOUND_LOADED){

#ifdef DOOMSOUND
		for (int i = 0; i < 16; i++) {
			Sfx_Stop(i);
		}
#else
		if (actrl) {
#ifdef PS3M
			//ps3mStop();
			ps3mUninitialize();
#endif

			struct AHIEffMasterVolume vol = {
				AHIET_MASTERVOLUME | AHIET_CANCEL,
				0x10000
			};
			AHI_SetEffect(&vol, actrl);

			AHI_FreeAudio(actrl);
			actrl = NULL;
		}
#endif

		if (sounds) {
			for (int i = 0; i < soundCount; i++) {
				free(sounds[i].sample);
			}
			free(sounds);
			sounds = NULL;
		}

		if (soundMap) {
			free(soundMap);
			soundMap = NULL;
		}

#ifdef PS3M
		if (musicData) {
			dRMemory_free((__POINTER__)musicData);
			musicData = NULL;
			musicSize = 0;
		}
#endif

		SOUND_LOADED = 0;
	}

	dRally_System_removeExitCallback(dRally_Sound_release);
}

void ___58b20h(unsigned int err_n, ...);
void ___42944h(const char *err);
__BYTE__ * S3M_getHeaderOrderList(s3m_t * s3m);

void dRally_Sound_load(__DWORD__ msx_t, const char * msx_f, __DWORD__ sfx_t, const char * sfx_f, __DWORD__ num_ch){

    int		n, size_s3m, size_xm;
	s3m_t *	musics_s3m;
	xm_t *	effects_xm;
	BPA * 	bpa;
	char 	err[0x50];

	printf("[dRally.SOUND] MSX: %s, SFX: %s\n", msx_f, sfx_f);

	dRally_Sound_release();

	bpa = bpa_open("MUSICS.BPA");

	bpa_search(bpa, msx_f);
	if((size_s3m = bpa_entry_size(bpa)) == 0){
        
        strcpy(err, "Problems with [");
        strcat(err, "MUSICS.BPA");
        strcat(err, "] ");
        strcat(err, msx_f);
        strcat(err, " file!");
		bpa_close(bpa);
        ___42944h(err);
    }
	else if((musics_s3m = (s3m_t *)dRMemory_alloc(size_s3m)) != (s3m_t *)0){
	
		//printf("music: %s\n", msx_f);
		bpa_read(bpa, (__POINTER__)musics_s3m);
		dREncryption_decodeCMF((__POINTER__)musics_s3m, size_s3m);
#ifdef PS3M
		musicChannels = 0;
		n = 32;
		while (--n > 0){
			if (musics_s3m->channelSettings[n] != 0xff) {
				musicChannels = n+1;
				break;
			}
		}
		//kprintf("music channels %ld music size %ld\n", musicChannels, size_s3m);
		musicData = musics_s3m;
		musicSize = size_s3m;
#else
		dRMemory_free((__POINTER__)musics_s3m);
		musics_s3m = NULL;
		size_s3m = 0;
#endif
	}

	bpa_search(bpa, sfx_f);
	if((size_xm = bpa_entry_size(bpa)) == 0){
        
        strcpy(err, "Problems with [");
        strcat(err, "MUSICS.BPA");
        strcat(err, "] ");
        strcat(err, sfx_f);
        strcat(err, " file!");
		bpa_close(bpa);
        ___42944h(err);
    }
	else if((effects_xm = (xm_t *)dRMemory_alloc(size_xm)) != (xm_t *)0){

		//printf("sound: %s\n", sfx_f);
		bpa_read(bpa, (__POINTER__)effects_xm);
		dREncryption_decodeCMF((__POINTER__)effects_xm, size_xm);
		effects_xm->version = SDL_SwapLE16(effects_xm->version);
		effects_xm->headerSize = SDL_SwapLE32(effects_xm->headerSize);
		effects_xm->songLength = SDL_SwapLE16(effects_xm->songLength);
		effects_xm->restartPosition = SDL_SwapLE16(effects_xm->restartPosition);
		effects_xm->channels = SDL_SwapLE16(effects_xm->channels);
		effects_xm->patterns = SDL_SwapLE16(effects_xm->patterns);
		effects_xm->instrumentCount = SDL_SwapLE16(effects_xm->instrumentCount);
		effects_xm->flags = SDL_SwapLE16(effects_xm->flags);//printf("flags %u\n", effects_xm->flags);
		effects_xm->defaultTempo = SDL_SwapLE16(effects_xm->defaultTempo);
		effects_xm->defaultBPM = SDL_SwapLE16(effects_xm->defaultBPM);
		if(strncmp(effects_xm->id, "Extended Module: ", 0x11)) ___58b20h(0x28, sfx_f);
		if(effects_xm->sig1 != 0x1a) ___58b20h(0x28, sfx_f);
		if(effects_xm->version < 0x104) ___58b20h(0x28, sfx_f);
		for (int i = 0; i < effects_xm->patterns; i++) {
			xm_pattern_t* pattern = XM_getPattern(effects_xm, i);
			pattern->header_size = SDL_SwapLE32(pattern->header_size);
			pattern->rows = SDL_SwapLE16(pattern->rows);
			pattern->packed_size = SDL_SwapLE16(pattern->packed_size);
		}
		for (int i = 0; i < effects_xm->instrumentCount; i++) {
			xm_instrument_t* inst = XM_getInstrument(effects_xm, i);
			inst->size = SDL_SwapLE32(inst->size);
			inst->n_samples = SDL_SwapLE16(inst->n_samples);
			inst->sample_header_size = SDL_SwapLE32(inst->sample_header_size);
			if (inst->n_samples > 0)
			{
				xm_sample_t* samp = XM_getInstrumentSamples(inst);
				samp->size = SDL_SwapLE32(samp->size);
				samp->loop_start = SDL_SwapLE32(samp->loop_start);
				samp->loop_length = SDL_SwapLE32(samp->loop_length);
				//printf("instrument %2d sounds %d size %d loop start %d loop length %d\n", i, inst->n_samples, samp->size, samp->loop_start, samp->loop_length);
				if (samp->type & 0x10) {
					__WORD__* data = (__WORD__*)XM_getInstrumentSamplesData(inst);
					for (int j = 0; j < samp->size / 2; j++) {
						data[j] = SDL_SwapLE16(data[j]);
					}
				}
			}
		}

		soundMap = malloc(effects_xm->instrumentCount * sizeof(int));
		memset(soundMap, -1, effects_xm->instrumentCount * sizeof(int));
		soundCount = 0;
		for (int i = 0; i < effects_xm->instrumentCount; i++) {
			xm_instrument_t* inst = XM_getInstrument(effects_xm, i);
			if (inst->n_samples > 0)
			{
				xm_sample_t* samp = XM_getInstrumentSamples(inst);
				if (samp->size > 0)
				{
					soundMap[i] = soundCount;
					soundCount++;
				}
			}
		}
		sounds = malloc(soundCount * sizeof(sound_t));
		for (int i = 0; i < effects_xm->instrumentCount; i++) {
			xm_instrument_t* inst = XM_getInstrument(effects_xm, i);
			int soundNum = soundMap[i];
			if (soundNum != -1)
			{
				xm_sample_t* samp = XM_getInstrumentSamples(inst);
				sound_t *sound = &sounds[soundNum];
				int samples, start;
				if ((samp->type & 3) != 0 && samp->loop_length > 0) {
					// looped sample
					start = samp->loop_start;
					samples = samp->loop_length;
					//printf("looped length %d\n", samples);
					if ((samp->type & 3) == 2) {
						// Ping-pong loop
						samples *= 2;
						//printf("bidi length %d\n", samples);
					}
					sound->loop = TRUE;
				} else {
					// no looping use the whole sample
					start = 0;
					samples = samp->size;
					//printf("no loop length %d\n", samples);
					sound->loop = FALSE;
				}

				if (samp->type & 0x10) {
					// 16-bit
					samples /= 2;
					sound->sample = malloc(samples);
					__WORD__* data = (__WORD__*)XM_getInstrumentSamplesData(inst);
					int old = 0;
					int k = 0;
					for (int j = 0; j < samp->size / 2; j++) {
						old += data[j];
						if (j >= start && k < samples) {
							sound->sample[k++] = old >> 8;
						}
					}
				} else {
					// 8-bit
					sound->sample = malloc(samples);
					__BYTE__* data = (__BYTE__*)XM_getInstrumentSamplesData(inst);
					int old = 0;
					int k = 0;
					for (int j = 0; j < samp->size; j++) {
						old += data[j];
						if (j >= start && k < samples) {
							sound->sample[k++] = old;
						}
					}
				}

				if ((samp->type & 3) == 2) {
					// create the mirrored half for bidirectional loops
					int halflength = samples / 2;
					for (int j = 0; j < halflength; j++) {
						sound->sample[j + halflength - 1] = sound->sample[j];
					}
				}

				sound->length = samples;
				/*
				sound->volume = samp->volume<<0xa;
				printf("volume %08x\n", sound->volume);
				*/
#if 1
				int period = 7680 - ((samp->rel_note + 49 - 1) * 64) - (samp->finetune / 2);
#else
				int period = 7680 - ((samp->rel_note + 49) * 64) - (samp->finetune / 2);
#endif
				sound->frequency = (int)(8363.0 * pow(2, (4608 - period) / 768.0));
				//printf("intr %d frequency %u rel_note %d finetune %d period %d\n", i, sound->frequency, samp->rel_note, samp->finetune, period);
			}
		}
		soundChannels = effects_xm->channels;

		//printf("instrumentCount %d channelCount %d soundCount %d\n", effects_xm->instrumentCount, effects_xm->channels, soundCount);

		dRMemory_free((__POINTER__)effects_xm);
		effects_xm = NULL;
		size_xm = 0;
	}

	bpa_close(bpa);

#ifdef DOOMSOUND
	/*
	for (int i = 0; i < soundCount; i++) {
		sound_t *sound = &sounds[i];
		for (int j = 0; j < sound->length; j++) {
			sound->sample[j] ^= 128;
		}
	}
	*/
	SOUND_LOADED = 1;
#else
//soundChannels = 1; // HACK!
	int ahiChannels = soundChannels;
	int ahiSounds = soundCount;

#ifdef PS3M
	ahiChannels += musicChannels;
	ahiSounds++;
	//kprintf("ahiSounds %ld ahiChannels %ld\n", ahiSounds, ahiChannels);
#endif

	actrl = AHI_AllocAudio(
		AHIA_AudioID, audioID,
		AHIA_MixFreq, SOUND_SAMPLERATE,
		AHIA_Channels, ahiChannels,
		AHIA_Sounds, ahiSounds,
#ifdef PS3M
		AHIA_SoundFunc, (IPTR)&SoundHook,
		AHIA_PlayerFunc, (IPTR)&PlayerHook,
		AHIA_PlayerFreq, (50 << 16),
		AHIA_MinPlayerFreq, (50 << 16),
		AHIA_MaxPlayerFreq, (50 << 16),
#endif
		//AHIA_UserData, AHI_SampleFrameSize(type),
		TAG_DONE);

	if (actrl) {
		for (int i = 0; i < soundCount; i++) {
			ULONG error;
			struct AHISampleInfo sample;

			sound_t *sound = &sounds[i];
			sample.ahisi_Address = sound->sample;
			sample.ahisi_Type = AHIST_M8S;
			sample.ahisi_Length = sound->length;
			error = AHI_LoadSound(i, AHIST_SAMPLE, &sample, actrl);

			//printf("loaded sound %2d length %d freq %d error %u\n", i, sound->length, sound->frequency, error);
#if 0
			{
				// dump the sample as a Sun AU file
				char filename[256];
				FILE *fd;
				struct
				{
					uint32_t magic; /* magic number */
					uint32_t hdr_size; /* size of this header */ 
					uint32_t data_size; /* length of data (optional) */ 
					uint32_t encoding; /* data encoding format */
					uint32_t sample_rate; /* samples per second */
					uint32_t channels; /* number of interleaved channels */
				} header;
				header.magic = SDL_SwapBE32(0x2e736e64);
				header.hdr_size = SDL_SwapBE32(sizeof(header));
				header.data_size = SDL_SwapBE32(0xffffffff);
				header.encoding = SDL_SwapBE32(2);
				header.sample_rate = SDL_SwapBE32(sound->frequency);
				header.channels = SDL_SwapBE32((uint32_t)1);

				snprintf(filename, sizeof(filename), "sound%02d.au", i);
				fd = fopen(filename, "w");
				fwrite(&header, sizeof(header), 1, fd);
				fwrite(sound->sample, sound->length, 1, fd);
				fclose(fd);
			}
#endif
		}

#ifdef PS3M
		//kprintf("soundCount %ld soundChannels %ld\n", soundCount, soundChannels);
		ps3mInitialize(musicSize, soundCount, soundChannels, musicData, AHIBase, actrl, (HOOKFUNC *)&PlayerHook.h_SubEntry, (HOOKFUNC *)&SoundHook.h_SubEntry);
		//kprintf("playerfunc %lx soundfunc %lx\n", PlayerHook.h_SubEntry, SoundHook.h_SubEntry);
#endif

		int r = soundChannels/2;
		//printf("master volume: %d -> %08x\n", r, r * 0x10000);

		struct AHIEffMasterVolume vol = {
			AHIET_MASTERVOLUME,
			r * 0x10000
		};
		AHI_SetEffect(&vol, actrl);
	}

	SOUND_LOADED = (actrl != NULL) ? 1 : 0; // TODO memory leak?
#endif

	MASTER_VOLUME = MSX_VOLUME = SFX_VOLUME = 0x10000;
	___65788h_updateVolume_cdecl();
	dRally_System_addExitCallback(dRally_Sound_release);
}

// 000648d8h
void dRally_Sound_play(void){

	__DWORD__ 	i, n;

	if(SOUND&&SOUND_LOADED){
		
		memset(___24e750h, 0xffffffff, 0x40);
#ifdef DOOMSOUND
		// nothing to do here
#else
		AHI_ControlAudio(actrl, AHIC_Play, TRUE, TAG_END);
#endif

		SOUND_PLAYING = 1;
		dRally_System_addExitCallback(dRally_Sound_stop);
	}
}

// 000655b0h
void dRally_Sound_adjustEffect(__BYTE__ channel, __DWORD__ vol, __DWORD__ freq, __DWORD__ balance){

	sound_t *sound;
	int n, soundNum;

	if(SOUND&&SOUND_LOADED&&channel){

		channel--;
		n = ___24e750h[channel];
		if (n != 0xffff){
			soundNum = soundMap[n-1];
			sound = &sounds[soundNum];
			freq = MULSHIFT(sound->frequency, freq);
			//freq = sound->frequency; // TODO remove
			vol = MULSHIFT(sfx_v, vol);

			//printf("adjust channel %2d sound %2d -> %2d freq %5d vol %08x balance %08x\n", channel, n, soundNum, freq, vol, balance);
#ifdef DOOMSOUND
			/*
			sep: 0-255, 128 center
			vol: 0-127
			*/
			vol = vol / 512 - 1;
			balance /= 256;
			Sfx_UpdatePitch(channel, freq);
			Sfx_UpdateVol(channel, vol, balance);
#else
			AHI_SetFreq(channel, freq, actrl, AHISF_IMM);
			AHI_SetVol(channel, vol, balance, actrl, AHISF_IMM);
#endif
		}
	}
}

// 000654d4h
void dRally_Sound_pushEffect(__BYTE__ sfx_channel, __BYTE__ n, __DWORD__ offset, __DWORD__ vol, __DWORD__ freq, __DWORD__ balance){

	sound_t *sound;
	int soundNum;

	if(SOUND&&SOUND_LOADED&&sfx_channel){

		sfx_channel--;
		soundNum = soundMap[n-1];
		sound = &sounds[soundNum];
#if 0
		if (sound->loop && ___24e750h[sfx_channel] == n)
			return;
#endif
		freq = MULSHIFT(sound->frequency, freq);
		//freq = sound->frequency; // TODO remove
		vol = MULSHIFT(sfx_v, vol);

#ifdef DOOMSOUND
		vol = vol / 512 - 1;
		balance /= 256;
		//printf("push channel %2d sound %2d -> %2d freq %5d vol %08x balance %08x\n", sfx_channel, n, soundNum, freq, vol, balance);
		//kprintf("dRally_Sound_pushEffect %ld\n", sfx_channel);
		Sfx_StartLoop(sound->sample - 8, sfx_channel, freq, vol, balance, sound->length, sound->loop ? 0 : -1);
#else
		//printf("push channel %2d sound %2d -> %2d freq %5d vol %08x balance %08x\n", sfx_channel, n, soundNum, freq, vol, balance);
		//kprintf("push channel %ld n %ld vol %lx loop %ld length %ld freq %ld\n", sfx_channel+1, n, vol, sound->loop, sound->length, freq);
		AHI_SetFreq(sfx_channel, freq, actrl, AHISF_IMM);
		AHI_SetVol(sfx_channel, vol, balance, actrl, AHISF_IMM);
		AHI_SetSound(sfx_channel, soundNum, offset, sound->length, actrl, AHISF_IMM);
		if (!sound->loop)
			AHI_SetSound(sfx_channel, AHI_NOSOUND, 0, 0, actrl, AHISF_NONE);
#endif
		___24e750h[sfx_channel] = n;
	}
}

// 00065990h
void dRally_Sound_setSampleRate(__DWORD__ freq){

}

// 0006563ch
void dRally_Sound_freeEffectChannel(__BYTE__ ch_num){

	if(SOUND&&SOUND_LOADED&&ch_num){

		ch_num--;
		//printf("free channel %2d\n", ch_num);
#ifdef DOOMSOUND
		Sfx_Stop(ch_num);
#else
		//kprintf("free channel %ld n %ld \n", ch_num+1, ___24e750h[ch_num]);
		AHI_SetSound(ch_num, AHI_NOSOUND, 0, 0, actrl, AHISF_IMM);
#endif
		___24e750h[ch_num] = 0xffff;
	}
}

// 000658b0h
__DWORD__ dRally_Sound_getPosition(void){

#ifdef PS3M
	ULONG row, order;
	row = ps3mGetRowPosition();
	order = ps3mGetSongPosition();
	return (order << 8 | row);
#else
	return 0xffffffff; // TODO PS3M position
#endif
}

// 000658b8h
__BYTE__ dRally_Sound_setPosition(__DWORD__ pos_n){

#ifdef PS3M
	//kprintf("dRally_Sound_setPosition(%lx) order %ld row %ld\n", pos_n, pos_n >> 8, pos_n & 0xFF);
	ps3mSetSongAndRowPosition(pos_n >> 8, pos_n & 0xFF);
	return 0;
#endif
	return 0; // the return value is ignored
}
