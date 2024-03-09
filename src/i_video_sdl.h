/*
 * i_video_sdl.h - Copyright (c) 2024-2025 - Olivier Poncet
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
#ifndef __I_VIDEO_SDL__
#define __I_VIDEO_SDL__

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include "doomstat.h"
#include "doomdef.h"
#include "i_system.h"
#include "i_video.h"
#include "v_video.h"
#include "d_main.h"
#include "m_argv.h"
#include "w_wad.h"
#include "z_zone.h"

// ---------------------------------------------------------------------------
// forward declarations
// ---------------------------------------------------------------------------

typedef struct I_VideoDriverRec I_VideoDriver;

// ---------------------------------------------------------------------------
// I_VideoDriver instance
// ---------------------------------------------------------------------------

struct I_VideoDriverRec
{
    SDL_Window*   window;
    SDL_Renderer* renderer;
    SDL_Surface*  surface;
    SDL_Palette*  palette;
    SDL_Surface*  picture;
    SDL_Texture*  texture;
    SDL_Texture*  overlay;
    void*         screen0;
    void*         screen1;
    void*         screen2;
    void*         screen3;
    void*         screen4;
    int           screen_w;
    int           screen_h;
    int           window_x;
    int           window_y;
    int           window_w;
    int           window_h;
    int           center_x;
    int           center_y;
    int           mouse_b;
    int           mouse_x;
    int           mouse_y;
    int           touch_b;
    int           touch_x;
    int           touch_y;
    int           touch_t;
    int           refcount;
};

// ---------------------------------------------------------------------------
// I_VideoDriver video interface
// ---------------------------------------------------------------------------

extern void I_VideoDriver_Video_Init          (I_VideoDriver* self);
extern void I_VideoDriver_Video_Fini          (I_VideoDriver* self);
extern void I_VideoDriver_Video_DoNothing     (I_VideoDriver* self);
extern void I_VideoDriver_Video_StartFrame    (I_VideoDriver* self);
extern void I_VideoDriver_Video_ProcessEvents (I_VideoDriver* self);
extern void I_VideoDriver_Video_UpdateFrame   (I_VideoDriver* self);
extern void I_VideoDriver_Video_RenderFrame   (I_VideoDriver* self);
extern void I_VideoDriver_Video_ReadScreen    (I_VideoDriver* self, byte* screen);
extern void I_VideoDriver_Video_SetPalette    (I_VideoDriver* self, byte* palette);

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------

#endif
