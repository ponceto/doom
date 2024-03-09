/*
 * i_sound_sdl.c - Copyright (c) 2024-2025 - Olivier Poncet
 *
 * DOOM - Copyright (C) 1993-1996 by id Software, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include "i_sound_sdl.h"

// ---------------------------------------------------------------------------
// some useful stuff
// ---------------------------------------------------------------------------

#ifndef countof
#define countof(array) (sizeof(array) / sizeof(array[0]))
#endif

#define FREQUENCY 44100
#define FLT32(value) ((float)(value))
#define INT32(value) ((int32_t)(value))

// ---------------------------------------------------------------------------
// I_AudioDriver global instance
// ---------------------------------------------------------------------------

static I_AudioDriver g_audio_driver = {
    FREQUENCY,      /* frequency  */
    AUDIO_F32SYS,   /* format     */
    2,              /* channels   */
    FREQUENCY / 35, /* chunksize  */
    0,              /* sound_id   */
    0,              /* music_id   */
    /* chunks */ {
        NULL, /* sfx_None   */
        NULL, /* sfx_pistol */
        NULL, /* sfx_shotgn */
        NULL, /* sfx_sgcock */
        NULL, /* sfx_dshtgn */
        NULL, /* sfx_dbopn  */
        NULL, /* sfx_dbcls  */
        NULL, /* sfx_dbload */
        NULL, /* sfx_plasma */
        NULL, /* sfx_bfg    */
        NULL, /* sfx_sawup  */
        NULL, /* sfx_sawidl */
        NULL, /* sfx_sawful */
        NULL, /* sfx_sawhit */
        NULL, /* sfx_rlaunc */
        NULL, /* sfx_rxplod */
        NULL, /* sfx_firsht */
        NULL, /* sfx_firxpl */
        NULL, /* sfx_pstart */
        NULL, /* sfx_pstop  */
        NULL, /* sfx_doropn */
        NULL, /* sfx_dorcls */
        NULL, /* sfx_stnmov */
        NULL, /* sfx_swtchn */
        NULL, /* sfx_swtchx */
        NULL, /* sfx_plpain */
        NULL, /* sfx_dmpain */
        NULL, /* sfx_popain */
        NULL, /* sfx_vipain */
        NULL, /* sfx_mnpain */
        NULL, /* sfx_pepain */
        NULL, /* sfx_slop   */
        NULL, /* sfx_itemup */
        NULL, /* sfx_wpnup  */
        NULL, /* sfx_oof    */
        NULL, /* sfx_telept */
        NULL, /* sfx_posit1 */
        NULL, /* sfx_posit2 */
        NULL, /* sfx_posit3 */
        NULL, /* sfx_bgsit1 */
        NULL, /* sfx_bgsit2 */
        NULL, /* sfx_sgtsit */
        NULL, /* sfx_cacsit */
        NULL, /* sfx_brssit */
        NULL, /* sfx_cybsit */
        NULL, /* sfx_spisit */
        NULL, /* sfx_bspsit */
        NULL, /* sfx_kntsit */
        NULL, /* sfx_vilsit */
        NULL, /* sfx_mansit */
        NULL, /* sfx_pesit  */
        NULL, /* sfx_sklatk */
        NULL, /* sfx_sgtatk */
        NULL, /* sfx_skepch */
        NULL, /* sfx_vilatk */
        NULL, /* sfx_claw   */
        NULL, /* sfx_skeswg */
        NULL, /* sfx_pldeth */
        NULL, /* sfx_pdiehi */
        NULL, /* sfx_podth1 */
        NULL, /* sfx_podth2 */
        NULL, /* sfx_podth3 */
        NULL, /* sfx_bgdth1 */
        NULL, /* sfx_bgdth2 */
        NULL, /* sfx_sgtdth */
        NULL, /* sfx_cacdth */
        NULL, /* sfx_skldth */
        NULL, /* sfx_brsdth */
        NULL, /* sfx_cybdth */
        NULL, /* sfx_spidth */
        NULL, /* sfx_bspdth */
        NULL, /* sfx_vildth */
        NULL, /* sfx_kntdth */
        NULL, /* sfx_pedth  */
        NULL, /* sfx_skedth */
        NULL, /* sfx_posact */
        NULL, /* sfx_bgact  */
        NULL, /* sfx_dmact  */
        NULL, /* sfx_bspact */
        NULL, /* sfx_bspwlk */
        NULL, /* sfx_vilact */
        NULL, /* sfx_noway  */
        NULL, /* sfx_barexp */
        NULL, /* sfx_punch  */
        NULL, /* sfx_hoof   */
        NULL, /* sfx_metal  */
        NULL, /* sfx_chgun  */
        NULL, /* sfx_tink   */
        NULL, /* sfx_bdopn  */
        NULL, /* sfx_bdcls  */
        NULL, /* sfx_itmbk  */
        NULL, /* sfx_flame  */
        NULL, /* sfx_flamst */
        NULL, /* sfx_getpow */
        NULL, /* sfx_bospit */
        NULL, /* sfx_boscub */
        NULL, /* sfx_bossit */
        NULL, /* sfx_bospn  */
        NULL, /* sfx_bosdth */
        NULL, /* sfx_manatk */
        NULL, /* sfx_mandth */
        NULL, /* sfx_sssit  */
        NULL, /* sfx_ssdth  */
        NULL, /* sfx_keenpn */
        NULL, /* sfx_keendt */
        NULL, /* sfx_skeact */
        NULL, /* sfx_skesit */
        NULL, /* sfx_skeatk */
        NULL, /* sfx_radio  */
    },
    0,        /* refcount   */
};

// ---------------------------------------------------------------------------
// Channels
// ---------------------------------------------------------------------------

static int g_channel_to_sound_id[16] = {
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
};

static void I_AudioDriver_Channel_Finished(int channel)
{
    const int min_channel = 0;
    const int max_channel = countof(g_channel_to_sound_id);

    if((channel >= min_channel) && (channel <= max_channel)) {
        g_channel_to_sound_id[channel] = -1;
    }
}

static void I_AudioDriver_Channel_Play(const int sound_id, Mix_Chunk* chunk)
{
    const int min_channel = 0;
    const int max_channel = countof(g_channel_to_sound_id);
    const int channel     = Mix_PlayChannel(-1, chunk, 0);

    if((channel >= min_channel) && (channel <= max_channel)) {
        g_channel_to_sound_id[channel] = sound_id;
    }
}

static void I_AudioDriver_Channel_Stop(const int sound_id)
{
    const int min_channel = 0;
    const int max_channel = countof(g_channel_to_sound_id);

    for(int channel = min_channel; channel < max_channel; ++channel) {
        if(g_channel_to_sound_id[channel] == sound_id) {
            g_channel_to_sound_id[channel] = (Mix_HaltChannel(channel), -1);
            break;
        }
    }
}

static int I_AudioDriver_Channel_IsPlaying(const int sound_id)
{
    const int min_channel = 0;
    const int max_channel = countof(g_channel_to_sound_id);

    for(int channel = min_channel; channel < max_channel; ++channel) {
        if(g_channel_to_sound_id[channel] == sound_id) {
            if(Mix_Playing(channel) == 0) {
                g_channel_to_sound_id[channel] = -1;
                return 0;
            }
            return 1;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// SoundData
// ---------------------------------------------------------------------------

typedef struct _SoundData SoundData;

struct _SoundData
{
    uint16_t format;
    uint16_t frequency;
    uint32_t length;
    uint8_t  padding[16];
    uint8_t  samples[16];
};

#ifndef SOUND_DATA
#define SOUND_DATA(data) ((SoundData*)(data))
#endif

// ---------------------------------------------------------------------------
// byte swapping utilities
// ---------------------------------------------------------------------------

static uint16_t le16(const uint16_t value)
{
#ifdef __BIG_ENDIAN__
    return ((value & 0xff00) >> 8)
         | ((value & 0x00ff) << 8)
         ;
#else
    return value;
#endif
}

static uint32_t le32(const uint32_t value)
{
#ifdef __BIG_ENDIAN__
    return ((value & 0xff000000) >> 24)
         | ((value & 0x00ff0000) >>  8)
         | ((value & 0x0000ff00) <<  8)
         | ((value & 0x000000ff) << 24)
         ;
#else
    return value;
#endif
}

// ---------------------------------------------------------------------------
// I_AudioDriver private interface
// ---------------------------------------------------------------------------

static void I_AudioDriver_InitBegin(I_AudioDriver* self)
{
    I_Debug("I_Sound: Initializing...");
}

static void I_AudioDriver_InitEnd(I_AudioDriver* self)
{
    I_Debug("I_Sound: Initialized!");
}

static void I_AudioDriver_FiniBegin(I_AudioDriver* self)
{
    I_Debug("I_Sound: Finalizing...");
}

static void I_AudioDriver_FiniEnd(I_AudioDriver* self)
{
    I_Debug("I_Sound: Finalized!");
}

static void I_AudioDriver_InitSystem(I_AudioDriver* self)
{
    const int rc = SDL_Init(0);

    if(rc != 0) {
        I_Error("I_Sound: SDL_Init() has failed");
    }
    else {
        I_Debug("I_Sound: SDL_Init() has succeeded");
    }
}

static void I_AudioDriver_FiniSystem(I_AudioDriver* self)
{
    const int rc = SDL_WasInit(SDL_INIT_AUDIO | SDL_INIT_VIDEO) != 0;

    if(rc == 0) {
        SDL_Quit();
        I_Debug("I_Sound: SDL_Quit() has succeeded");
    }
}

static void I_AudioDriver_InitSubSystem(I_AudioDriver* self)
{
    const int rc = SDL_InitSubSystem(SDL_INIT_AUDIO);

    if(rc != 0) {
        I_Error("I_Sound: SDL_InitSubSystem() has failed");
    }
    else {
        I_Debug("I_Sound: SDL_InitSubSystem() has succeeded");
    }
}

static void I_AudioDriver_FiniSubSystem(I_AudioDriver* self)
{
    const int rc = (SDL_QuitSubSystem(SDL_INIT_AUDIO), 0);

    if(rc != 0) {
        I_Error("I_Sound: SDL_QuitSubSystem() has failed");
    }
    else {
        I_Debug("I_Sound: SDL_QuitSubSystem() has succeeded");
    }
}

static void I_AudioDriver_InitDevice(I_AudioDriver* self)
{
    const int frequency = self->frequency;
    const int format    = self->format;
    const int channels  = self->channels;
    const int chunksize = self->chunksize;
    const int rc = Mix_OpenAudio(frequency, format, channels, chunksize);

    if(rc != 0) {
        I_Error("I_Sound: Mix_OpenAudio() has failed");
    }
    else {
        I_Debug("I_Sound: Mix_OpenAudio() has succeeded");
    }
    Mix_ChannelFinished(&I_AudioDriver_Channel_Finished);
}

static void I_AudioDriver_FiniDevice(I_AudioDriver* self)
{
    const int rc = (Mix_CloseAudio(), 0);

    if(rc != 0) {
        I_Error("I_Sound: Mix_CloseAudio() has failed");
    }
    else {
        I_Debug("I_Sound: Mix_CloseAudio() has succeeded");
    }
}

static void I_AudioDriver_InitChunks(I_AudioDriver* self)
{
    const int count = countof(self->chunks);
    for(int index = 0; index < count; ++index) {
        self->chunks[index] = NULL;
    }
}

static void I_AudioDriver_FiniChunks(I_AudioDriver* self)
{
    const int count = countof(self->chunks);
    for(int index = 0; index < count; ++index) {
        Mix_Chunk* chunk = self->chunks[index];
        if(chunk != NULL) {
            self->chunks[index] = (Mix_FreeChunk(chunk), NULL);
            I_Debug("I_Sound: Mix_FreeChunk() has succeeded");
        }
    }
}

static void I_AudioDriver_InitChannels(I_AudioDriver* self)
{
    const int count = countof(g_channel_to_sound_id);
    for(int index = 0; index < count; ++index) {
        g_channel_to_sound_id[index] = -1;
    }
}

static void I_AudioDriver_FiniChannels(I_AudioDriver* self)
{
    const int count = countof(g_channel_to_sound_id);
    for(int index = 0; index < count; ++index) {
        if(g_channel_to_sound_id[index] != -1) {
            g_channel_to_sound_id[index] = (Mix_HaltChannel(index), -1);
        }
    }
}

static void I_AudioDriver_Init(I_AudioDriver* self)
{
    if(self->refcount++ == 0) {
        I_AudioDriver_InitBegin(self);
        I_AudioDriver_InitSystem(self);
        I_AudioDriver_InitSubSystem(self);
        I_AudioDriver_InitDevice(self);
        I_AudioDriver_InitChunks(self);
        I_AudioDriver_InitChannels(self);
        I_AudioDriver_InitEnd(self);
    }
}

static void I_AudioDriver_Fini(I_AudioDriver* self)
{
    if(--self->refcount == 0) {
        I_AudioDriver_FiniBegin(self);
        I_AudioDriver_FiniChannels(self);
        I_AudioDriver_FiniChunks(self);
        I_AudioDriver_FiniDevice(self);
        I_AudioDriver_FiniSubSystem(self);
        I_AudioDriver_FiniSystem(self);
        I_AudioDriver_FiniEnd(self);
    }
}

static void I_AudioDriver_Resample(I_AudioDriver* self, float* dstptr, uint32_t dstlen, const uint8_t* srcptr, uint32_t srclen)
{
#if 0 /* simple and fast bresenham style algorithm */
    if((dstlen != 0) && (srclen != 0) && (srclen <= dstlen)) {
        uint32_t count = dstlen;
        uint32_t error = 0;
        do {
            *dstptr++ = (FLT32(*srcptr) - 128.0f) / 128.0f;
            error += srclen;
            if(error >= dstlen) {
                error -= dstlen;
                ++srcptr;
            }
        } while(--count != 0);
    }
    else {
        I_Alert("unable to resample sound");
    }
#else /* fixed point algorithm with interpolation */
    if((dstlen != 0) && (srclen != 0)) {
        uint32_t count  = dstlen;
        uint32_t srcpos = 0;
        uint32_t srcinc = ((srclen - 1) << 8) / dstlen;
        do {
            const uint32_t sp = (srcpos >> 8);                /* get the position integer part    */
            const int32_t  sr = (srcpos & 0xff);              /* get the position fractional part */
            const int32_t  i1 = (0xff - sr);                  /* compute 1st interpolation factor */
            const int32_t  i2 = (0x00 + sr);                  /* compute 2nd interpolation factor */
            const int32_t  s1 = INT32(srcptr[sp + 0]) - 128;  /* get the 1st sample               */
            const int32_t  s2 = INT32(srcptr[sp + 1]) - 128;  /* get the 2nd sample               */
            const int32_t  s3 = ((s1 * i1) + (s2 * i2)) >> 8; /* interpolate the two samples      */
            *dstptr++ = FLT32(s3) / 128.0f;
            srcpos += srcinc;
        } while(--count != 0);
    }
    else {
        I_Alert("unable to resample sound");
    }
#endif
}

// ---------------------------------------------------------------------------
// I_AudioDriver sound interface
// ---------------------------------------------------------------------------

void I_AudioDriver_Sound_Init(I_AudioDriver* self)
{
    I_AudioDriver_Init(self);
}

void I_AudioDriver_Sound_Fini(I_AudioDriver* self)
{
    I_AudioDriver_Fini(self);
}

void I_AudioDriver_Sound_DoNothing(I_AudioDriver* self)
{
}

void I_AudioDriver_Sound_Play(I_AudioDriver* self, const int sfx_id, const int sound_id, const int volume, const int pan, const int pitch, const int priority)
{
    sfxinfo_t* sfx = &S_sfx[sfx_id];

    I_Trace("I_Sound: Sound::Play(sfx_id=%d, sound_id=%d, volume=%d, pan=%d, pitch=%d, priority=%d)", sfx_id, sound_id, volume, pan, pitch, priority);
    /* cache lump if needed */ {
        if((sfx->lumpnum >= 0) && (sfx->data == NULL)) {
            sfx->data = W_CacheLumpNum(sfx->lumpnum, PU_SOUND);
            if(sfx->data == NULL) {
                return;
            }
        }
    }
    /* create chunk if needed */ {
        Mix_Chunk* chunk = self->chunks[sfx_id];
        if(chunk == NULL) {
            const SoundData* sound_data      = SOUND_DATA(sfx->data);
            const uint16_t   sound_format    = le16(sound_data->format);
            const uint16_t   sound_frequency = le16(sound_data->frequency);
            const uint32_t   sound_length    = le32(sound_data->length) - 32;
            const uint8_t*   sound_buffer    = sound_data->samples;
            const uint16_t   audio_frequency = self->frequency;
            if(sound_format == 0x0003) {
                const uint32_t srclen = sound_length;
                const uint8_t* srcptr = sound_buffer;
                const uint32_t dstlen = ((sound_length * audio_frequency) / sound_frequency);
                float*         dstptr = (float*) SDL_calloc((dstlen + 4) & ~3, sizeof(float));
                if(dstptr != NULL) {
                    I_AudioDriver_Resample(self, dstptr, dstlen, srcptr, srclen);
                }
                else {
                    I_Error("SDL_calloc has failed");
                }
                /* create chunk */ {
                    uint8_t*       buffer = ((uint8_t*)(dstptr));
                    const uint32_t length = (dstlen * sizeof(float));
                    chunk = Mix_QuickLoad_RAW(buffer, length);
                    if(chunk != NULL) {
                        chunk->allocated = 1;
                        chunk->volume    = ((volume << 3) | (volume >> 1));
                        self->chunks[sfx_id] = chunk;
                    }
                    else {
                        I_Error("Mix_QuickLoad_RAW has failed");
                    }
                }
            }
        }
        else {
            chunk->volume = ((volume << 3) | (volume >> 1));
        }
    }
    I_AudioDriver_Channel_Play(sound_id, self->chunks[sfx_id]);
}

void I_AudioDriver_Sound_Update(I_AudioDriver* self, const int sound_id, const int volume, const int pan, const int pitch)
{
    I_Trace("I_Sound: Sound::Update(sound_id=%d, volume=%d, pan=%d, pitch=%d)", sound_id, volume, pan, pitch);
}

void I_AudioDriver_Sound_Stop(I_AudioDriver* self, const int sound_id)
{
    I_Trace("I_Sound: Sound::Stop(sound_id=%d)", sound_id);

    I_AudioDriver_Channel_Stop(sound_id);
}

int I_AudioDriver_Sound_IsPlaying(I_AudioDriver* self, const int sound_id)
{
    I_Trace("I_Sound: Sound::IsPlaying(sound_id=%d)", sound_id);

    return I_AudioDriver_Channel_IsPlaying(sound_id);
}

// ---------------------------------------------------------------------------
// I_AudioDriver music interface
// ---------------------------------------------------------------------------

void I_AudioDriver_Music_Init(I_AudioDriver* self)
{
    I_AudioDriver_Init(self);
}

void I_AudioDriver_Music_Fini(I_AudioDriver* self)
{
    I_AudioDriver_Fini(self);
}

void I_AudioDriver_Music_DoNothing(I_AudioDriver* self)
{
}

void I_AudioDriver_Music_SetVolume(I_AudioDriver* self, const int volume)
{
    I_Trace("I_Sound: Music::SetVolume(volume=%d)", volume);
}

void I_AudioDriver_Music_Play(I_AudioDriver* self, const int music_id, int looping)
{
    I_Trace("I_Sound: Music::Play(music_id=%d, looping=%d)", music_id, looping);
}

void I_AudioDriver_Music_Pause(I_AudioDriver* self, const int music_id)
{
    I_Trace("I_Sound: Music::Pause(music_id=%d)", music_id);
}

void I_AudioDriver_Music_Resume(I_AudioDriver* self, const int music_id)
{
    I_Trace("I_Sound: Music::Resume(music_id=%d)", music_id);
}

void I_AudioDriver_Music_Stop(I_AudioDriver* self, const int music_id)
{
    I_Trace("I_Sound: Music::Stop(music_id=%d)", music_id);
}

void I_AudioDriver_Music_Register(I_AudioDriver* self, const int music_id, void* data)
{
    I_Trace("I_Sound: Music::Register(data=%p)", data);
}

void I_AudioDriver_Music_UnRegister(I_AudioDriver* self, const int music_id)
{
    I_Trace("I_Sound: Music::UnRegister(music_id=%d)", music_id);
}

// ---------------------------------------------------------------------------
// DOOM sound interface
// ---------------------------------------------------------------------------

void I_InitSound(void)
{
    I_AudioDriver_Sound_Init(&g_audio_driver);
}

void I_ShutdownSound(void)
{
    I_AudioDriver_Sound_Fini(&g_audio_driver);
}

void I_UpdateSound(void)
{
    I_AudioDriver_Sound_DoNothing(&g_audio_driver);
}

void I_SubmitSound(void)
{
    I_AudioDriver_Sound_DoNothing(&g_audio_driver);
}

void I_SetChannels(void)
{
    I_AudioDriver_Sound_DoNothing(&g_audio_driver);
}

int I_StartSound(int sfx_id, int volume, int pan, int pitch, int priority)
{
    const int sound_id = ++g_audio_driver.sound_id;

    if(sound_id > 0) {
        I_AudioDriver_Sound_Play(&g_audio_driver, sfx_id, sound_id, volume, pan, pitch, priority);
    }
    return sound_id;
}

void I_UpdateSoundParams(int sound_id, int volume, int pan, int pitch)
{
    I_AudioDriver_Sound_Update(&g_audio_driver, sound_id, volume, pan, pitch);
}

void I_StopSound(int sound_id)
{
    I_AudioDriver_Sound_Stop(&g_audio_driver, sound_id);
}

int I_SoundIsPlaying(int sound_id)
{
    return I_AudioDriver_Sound_IsPlaying(&g_audio_driver, sound_id);
}

int I_GetSfxLumpNum(sfxinfo_t* sfx)
{
    char buffer[256];
    int  buflen  = snprintf(buffer, sizeof(buffer), "ds%s", sfx->name);
    int  lumpnum = -1;

    if(buflen > 0) {
        lumpnum = W_GetNumForName(buffer);
    }
    else {
        I_Error("I_GetSfxLumpNum: snprintf() has failed");
    }
    return lumpnum;
}

// ---------------------------------------------------------------------------
// DOOM music interface
// ---------------------------------------------------------------------------

void I_InitMusic(void)
{
    I_AudioDriver_Music_Init(&g_audio_driver);
}

void I_ShutdownMusic(void)
{
    I_AudioDriver_Music_Fini(&g_audio_driver);
}

void I_SetMusicVolume(int volume)
{
    I_AudioDriver_Music_SetVolume(&g_audio_driver, volume);
}

void I_PlaySong(int music_id, int looping)
{
    I_AudioDriver_Music_Play(&g_audio_driver, music_id, looping);
}

void I_PauseSong(int music_id)
{
    I_AudioDriver_Music_Pause(&g_audio_driver, music_id);
}

void I_ResumeSong(int music_id)
{
    I_AudioDriver_Music_Resume(&g_audio_driver, music_id);
}

void I_StopSong(int mudic_id)
{
    I_AudioDriver_Music_Stop(&g_audio_driver, mudic_id);
}

int I_RegisterSong(void* data)
{
    const int music_id = ++g_audio_driver.music_id;

    if(music_id > 0) {
        I_AudioDriver_Music_Register(&g_audio_driver, music_id, data);
    }
    return music_id;
}

void I_UnRegisterSong(int music_id)
{
    I_AudioDriver_Music_UnRegister(&g_audio_driver, music_id);
}

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------
