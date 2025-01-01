/*
 * i_sound_sdl.h - Copyright (c) 2024-2025 - Olivier Poncet
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
#ifndef __I_SOUND_SDL__
#define __I_SOUND_SDL__

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "doomstat.h"
#include "doomdef.h"
#include "i_system.h"
#include "i_sound.h"
#include "s_sound.h"
#include "d_main.h"
#include "m_argv.h"
#include "w_wad.h"
#include "z_zone.h"
#include "sounds.h"

// ---------------------------------------------------------------------------
// forward declarations
// ---------------------------------------------------------------------------

typedef struct I_AudioDriverRec I_AudioDriver;

// ---------------------------------------------------------------------------
// I_AudioDriver instance
// ---------------------------------------------------------------------------

struct I_AudioDriverRec
{
    int        frequency;
    int        format;
    int        channels;
    int        chunksize;
    int        sound_id;
    int        music_id;
    Mix_Chunk* chunks[NUMSFX];
    int        refcount;
};

// ---------------------------------------------------------------------------
// I_AudioDriver sound interface
// ---------------------------------------------------------------------------

extern void I_AudioDriver_Sound_Init      (I_AudioDriver* self);
extern void I_AudioDriver_Sound_Fini      (I_AudioDriver* self);
extern void I_AudioDriver_Sound_DoNothing (I_AudioDriver* self);

// ---------------------------------------------------------------------------
// I_AudioDriver music interface
// ---------------------------------------------------------------------------

extern void I_AudioDriver_Music_Init      (I_AudioDriver* self);
extern void I_AudioDriver_Music_Fini      (I_AudioDriver* self);
extern void I_AudioDriver_Music_DoNothing (I_AudioDriver* self);

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------

#endif
