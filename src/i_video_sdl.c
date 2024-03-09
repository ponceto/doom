/*
 * i_video_sdl.c - Copyright (c) 2024 - Olivier Poncet
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
#include "i_video_sdl.h"

// ---------------------------------------------------------------------------
// some useful stuff
// ---------------------------------------------------------------------------

#ifndef countof
#define countof(array) (sizeof(array) / sizeof(array[0]))
#endif

// ---------------------------------------------------------------------------
// I_VideoDriver global instance
// ---------------------------------------------------------------------------

static I_VideoDriver g_video_driver = {
    NULL, /* window    */
    NULL, /* renderer  */
    NULL, /* surface   */
    NULL, /* palette   */
    NULL, /* picture   */
    NULL, /* texture   */
    NULL, /* screen0   */
    NULL, /* screen1   */
    NULL, /* screen2   */
    NULL, /* screen3   */
    NULL, /* screen4   */
    0,    /* screen_w  */
    0,    /* screen_h  */
    0,    /* window_x  */
    0,    /* window_y  */
    0,    /* window_w  */
    0,    /* window_h  */
    0,    /* center_x  */
    0,    /* center_y  */
    0,    /* mouse_b   */
    0,    /* mouse_x   */
    0,    /* mouse_y   */
    0,    /* refcount  */
};

// ---------------------------------------------------------------------------
// I_VideoDriver private interface
// ---------------------------------------------------------------------------

static void I_VideoDriver_InitBegin(I_VideoDriver* self)
{
    I_Debug("I_Video: Initializing...");

    /* parse command-line for scale factor */ {
        int argi = 0;
        int last = 0;
        /* scale factor: 1.0 */ {
            if((argi = M_CheckParm("-1")) && (argi > last)) {
                g_scale_mul = 1;
                g_scale_div = 1;
                last = argi;
            }
        }
        /* scale factor: 1.5 */ {
            if((argi = M_CheckParm("-2")) && (argi > last)) {
                g_scale_mul = 3;
                g_scale_div = 2;
                last = argi;
            }
        }
        /* scale factor: 2.0 */ {
            if((argi = M_CheckParm("-3")) && (argi > last)) {
                g_scale_mul = 2;
                g_scale_div = 1;
                last = argi;
            }
        }
        /* scale factor: 2.5 */ {
            if((argi = M_CheckParm("-4")) && (argi > last)) {
                g_scale_mul = 5;
                g_scale_div = 2;
                last = argi;
            }
        }
        /* scale factor: 3.0 */ {
            if((argi = M_CheckParm("-5")) && (argi > last)) {
                g_scale_mul = 3;
                g_scale_div = 1;
                last = argi;
            }
        }
        /* scale factor: 3.5 */ {
            if((argi = M_CheckParm("-6")) && (argi > last)) {
                g_scale_mul = 7;
                g_scale_div = 2;
                last = argi;
            }
        }
        /* scale factor: 4.0 */ {
            if((argi = M_CheckParm("-7")) && (argi > last)) {
                g_scale_mul = 4;
                g_scale_div = 1;
                last = argi;
            }
        }
        /* scale factor: 4.5 */ {
            if((argi = M_CheckParm("-8")) && (argi > last)) {
                g_scale_mul = 9;
                g_scale_div = 2;
                last = argi;
            }
        }
        /* scale factor: 5.0 */ {
            if((argi = M_CheckParm("-9")) && (argi > last)) {
                g_scale_mul = 5;
                g_scale_div = 1;
                last = argi;
            }
        }
        /* scale factor: sanity checks */ {
            if((g_scale_mul <= 0)
            || (g_scale_div <= 0)) {
                g_scale_mul = 3;
                g_scale_div = 1;
            }
        }
    }
    /* parse command-line for scale mode */ {
        int argi = 0;
        int last = 0;
        /* scale mode: nearest */ {
            if((argi = M_CheckParm("-nearest")) && (argi > last)) {
                g_scale_mode = SDL_ScaleModeNearest;
            }
        }
        /* scale mode: linear */ {
            if((argi = M_CheckParm("-linear")) && (argi > last)) {
                g_scale_mode = SDL_ScaleModeLinear;
            }
        }
        /* scale mode: best */ {
            if((argi = M_CheckParm("-best")) && (argi > last)) {
                g_scale_mode = SDL_ScaleModeBest;
            }
        }
        /* scale mode: sanity checks */ {
            switch(g_scale_mode) {
                case SDL_ScaleModeNearest:
                case SDL_ScaleModeLinear:
                case SDL_ScaleModeBest:
                    break;
                default:
                    g_scale_mode = SDL_ScaleModeBest;
                    break;
            }
        }
    }
}

static void I_VideoDriver_InitEnd(I_VideoDriver* self)
{
    I_Debug("I_Video: Initialized!");
}

static void I_VideoDriver_FiniBegin(I_VideoDriver* self)
{
    I_Debug("I_Video: Finalizing...");
}

static void I_VideoDriver_FiniEnd(I_VideoDriver* self)
{
    I_Debug("I_Video: Finalized!");
}

static void I_VideoDriver_InitSystem(I_VideoDriver* self)
{
    const int rc = SDL_Init(0);

    if(rc != 0) {
        I_Error("I_Video: SDL_Init() has failed");
    }
    else {
        I_Debug("I_Video: SDL_Init() has succeeded");
    }
}

static void I_VideoDriver_FiniSystem(I_VideoDriver* self)
{
    const int rc = SDL_WasInit(SDL_INIT_AUDIO | SDL_INIT_VIDEO) != 0;

    if(rc == 0) {
        SDL_Quit();
        I_Debug("I_Video: SDL_Quit() has succeeded");
    }
}

static void I_VideoDriver_InitSubSystem(I_VideoDriver* self)
{
    const int rc = SDL_InitSubSystem(SDL_INIT_VIDEO);

    if(rc != 0) {
        I_Error("I_Video: SDL_InitSubSystem() has failed");
    }
    else {
        I_Debug("I_Video: SDL_InitSubSystem() has succeeded");
    }
    self->screen_w  = SCREENWIDTH;
    self->screen_h  = SCREENHEIGHT;
    self->window_x  = SDL_WINDOWPOS_UNDEFINED;
    self->window_y  = SDL_WINDOWPOS_UNDEFINED;
    self->window_w  = ((self->screen_w * g_scale_mul) / g_scale_div);
    self->window_h  = ((self->screen_h * g_scale_mul) / g_scale_div);
    self->center_x  = (self->window_w / 2);
    self->center_y  = (self->window_h / 2);
    self->mouse_b   = 0;
    self->mouse_x   = self->center_x;
    self->mouse_y   = self->center_y;
}

static void I_VideoDriver_FiniSubSystem(I_VideoDriver* self)
{
    const int rc = (SDL_QuitSubSystem(SDL_INIT_VIDEO), 0);

    if(rc != 0) {
        I_Error("I_Video: SDL_QuitSubSystem() has failed");
    }
    else {
        I_Debug("I_Video: SDL_QuitSubSystem() has succeeded");
    }
}

static void I_VideoDriver_InitWindow(I_VideoDriver* self)
{
    const char*  title = g_title;
    const int    pos_x = self->window_x;
    const int    pos_y = self->window_y;
    const int    dim_w = self->window_w;
    const int    dim_h = self->window_h;
    const Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;

    if(self->window == NULL) {
        self->window = SDL_CreateWindow(title, pos_x, pos_y, dim_w, dim_h, flags);
    }
    if(self->window == NULL) {
        I_Error("I_Video: SDL_CreateWindow() has failed");
    }
    else {
        I_Debug("I_Video: SDL_CreateWindow() has succeeded");
    }
}

static void I_VideoDriver_FiniWindow(I_VideoDriver* self)
{
    if(self->window != NULL) {
        self->window = (SDL_DestroyWindow(self->window), NULL);
    }
}

static void I_VideoDriver_InitRenderer(I_VideoDriver* self)
{
    const int    index = -1;
    const Uint32 flags =  0;

    if(self->renderer == NULL) {
        self->renderer = SDL_CreateRenderer(self->window, index, flags);
    }
    if(self->renderer == NULL) {
        I_Error("I_Video: SDL_CreateRenderer() has failed");
    }
    else {
        I_Debug("I_Video: SDL_CreateRenderer() has succeeded");
    }
}

static void I_VideoDriver_FiniRenderer(I_VideoDriver* self)
{
    if(self->renderer != NULL) {
        self->renderer = (SDL_DestroyRenderer(self->renderer), NULL);
    }
}

static void I_VideoDriver_InitSurface(I_VideoDriver* self)
{
    const int    width  = self->screen_w;
    const int    height = self->screen_h;
    const int    depth  = 8;
    const Uint32 rmask  = 0;
    const Uint32 gmask  = 0;
    const Uint32 bmask  = 0;
    const Uint32 amask  = 0;
    const Uint32 flags  = 0;

    if(self->surface == NULL) {
        self->surface = SDL_CreateRGBSurface(flags, width, height, depth, rmask, gmask, bmask, amask);
    }
    if(self->surface == NULL) {
        I_Error("I_Video: SDL_CreateRGBSurface() has failed");
    }
    else {
        I_Debug("I_Video: SDL_CreateRGBSurface() has succeeded");
    }
}

static void I_VideoDriver_FiniSurface(I_VideoDriver* self)
{
    if(self->surface != NULL) {
        self->surface = (SDL_FreeSurface(self->surface), NULL);
    }
}

static void I_VideoDriver_InitPalette(I_VideoDriver* self)
{
    if(self->palette == NULL) {
        self->palette = SDL_AllocPalette(256);
    }
    if(self->palette == NULL) {
        I_Error("I_Video: SDL_AllocPalette() has failed");
    }
    else {
        I_Debug("I_Video: SDL_AllocPalette() has succeeded");
    }
    if(self->surface != NULL) {
        SDL_SetSurfacePalette(self->surface, self->palette);
    }
}

static void I_VideoDriver_FiniPalette(I_VideoDriver* self)
{
    if(self->palette != NULL) {
        self->palette = (SDL_FreePalette(self->palette), NULL);
    }
}

static void I_VideoDriver_InitPicture(I_VideoDriver* self)
{
    const int    width  = self->screen_w;
    const int    height = self->screen_h;
    const int    depth  = 32;
    const Uint32 rmask  = 0xff0000;
    const Uint32 gmask  = 0x00ff00;
    const Uint32 bmask  = 0x0000ff;
    const Uint32 amask  = 0;
    const Uint32 flags  = 0;

    if(self->picture == NULL) {
        self->picture = SDL_CreateRGBSurface(flags, width, height, depth, rmask, gmask, bmask, amask);
    }
    if(self->picture == NULL) {
        I_Error("I_Video: SDL_CreateRGBSurface() has failed");
    }
    else {
        I_Debug("I_Video: SDL_CreateRGBSurface() has succeeded");
    }
}

static void I_VideoDriver_FiniPicture(I_VideoDriver* self)
{
    if(self->picture != NULL) {
        self->picture = (SDL_FreeSurface(self->picture), NULL);
    }
}

static void I_VideoDriver_InitTexture(I_VideoDriver* self)
{
    if(self->texture == NULL) {
        const Uint32 format = SDL_PIXELFORMAT_RGB888;
        const int    access = SDL_TEXTUREACCESS_STREAMING;
        const int    width  = self->screen_w;
        const int    height = self->screen_h;
        self->texture = SDL_CreateTexture(self->renderer, format, access, width, height);
    }
    if(self->texture == NULL) {
        I_Error("I_Video: SDL_CreateTexture() has failed");
    }
    else {
        I_Debug("I_Video: SDL_CreateTexture() has succeeded");
    }
    if(self->texture != NULL) {
        SDL_SetTextureScaleMode(self->texture, g_scale_mode);
    }
}

static void I_VideoDriver_FiniTexture(I_VideoDriver* self)
{
    if(self->texture != NULL) {
        self->texture = (SDL_DestroyTexture(self->texture), NULL);
    }
}

static void I_VideoDriver_InitScreens(I_VideoDriver* self)
{
    /* backup all doom screens */ {
        self->screen0 = screens[0];
        self->screen1 = screens[1];
        self->screen2 = screens[2];
        self->screen3 = screens[3];
        self->screen4 = screens[4];
    }
    if(self->surface != NULL) {
        screens[0] = (unsigned char *) self->surface->pixels;
    }
}

static void I_VideoDriver_FiniScreens(I_VideoDriver* self)
{
    /* restore all doom screens */ {
        self->screen0 = ((screens[0] = self->screen0), NULL);
        self->screen1 = ((screens[1] = self->screen1), NULL);
        self->screen2 = ((screens[2] = self->screen2), NULL);
        self->screen3 = ((screens[3] = self->screen3), NULL);
        self->screen4 = ((screens[4] = self->screen4), NULL);
    }
}

static void I_VideoDriver_Init(I_VideoDriver* self)
{
    if(self->refcount++ == 0) {
        I_VideoDriver_InitBegin(self);
        I_VideoDriver_InitSystem(self);
        I_VideoDriver_InitSubSystem(self);
        I_VideoDriver_InitWindow(self);
        I_VideoDriver_InitRenderer(self);
        I_VideoDriver_InitSurface(self);
        I_VideoDriver_InitPalette(self);
        I_VideoDriver_InitPicture(self);
        I_VideoDriver_InitTexture(self);
        I_VideoDriver_InitScreens(self);
        I_VideoDriver_InitEnd(self);
    }
}

static void I_VideoDriver_Fini(I_VideoDriver* self)
{
    if(--self->refcount == 0) {
        I_VideoDriver_FiniBegin(self);
        I_VideoDriver_FiniScreens(self);
        I_VideoDriver_FiniTexture(self);
        I_VideoDriver_FiniPicture(self);
        I_VideoDriver_FiniPalette(self);
        I_VideoDriver_FiniSurface(self);
        I_VideoDriver_FiniRenderer(self);
        I_VideoDriver_FiniWindow(self);
        I_VideoDriver_FiniSubSystem(self);
        I_VideoDriver_FiniSystem(self);
        I_VideoDriver_FiniEnd(self);
    }
}

static int I_VideoDriver_TranslateKey(I_VideoDriver* self, const SDL_KeyboardEvent* event)
{
    int key = -1;

    switch(event->keysym.sym) {
        case SDLK_RIGHT:
            key = KEY_RIGHTARROW;
            break;
        case SDLK_LEFT:
            key = KEY_LEFTARROW;
            break;
        case SDLK_UP:
            key = KEY_UPARROW;
            break;
        case SDLK_DOWN:
            key = KEY_DOWNARROW;
            break;
        case SDLK_ESCAPE:
            key = KEY_ESCAPE;
            break;
        case SDLK_RETURN:
            key = KEY_ENTER;
            break;
        case SDLK_RETURN2:
            key = KEY_ENTER;
            break;
        case SDLK_KP_ENTER:
            key = KEY_ENTER;
            break;
        case SDLK_TAB:
            key = KEY_TAB;
            break;
        case SDLK_F1:
            key = KEY_F1;
            break;
        case SDLK_F2:
            key = KEY_F2;
            break;
        case SDLK_F3:
            key = KEY_F3;
            break;
        case SDLK_F4:
            key = KEY_F4;
            break;
        case SDLK_F5:
            key = KEY_F5;
            break;
        case SDLK_F6:
            key = KEY_F6;
            break;
        case SDLK_F7:
            key = KEY_F7;
            break;
        case SDLK_F8:
            key = KEY_F8;
            break;
        case SDLK_F9:
            key = KEY_F9;
            break;
        case SDLK_F10:
            key = KEY_F10;
            break;
        case SDLK_F11:
            key = KEY_F11;
            break;
        case SDLK_F12:
            key = KEY_F12;
            break;
        case SDLK_BACKSPACE:
            key = KEY_BACKSPACE;
            break;
        case SDLK_PAUSE:
            key = KEY_PAUSE;
            break;
        case SDLK_EQUALS:
            key = KEY_EQUALS;
            break;
        case SDLK_KP_EQUALS:
            key = KEY_EQUALS;
            break;
        case SDLK_KP_MINUS:
            key = KEY_MINUS;
            break;
        case SDLK_RSHIFT:
            key = KEY_RSHIFT;
            break;
        case SDLK_LCTRL:
            key = KEY_RCTRL;
            break;
        case SDLK_RCTRL:
            key = KEY_RCTRL;
            break;
        case SDLK_LALT:
            key = KEY_LALT;
            break;
        case SDLK_RALT:
            key = KEY_RALT;
            break;
        default:
            if((event->keysym.sym >= 0x20)
            && (event->keysym.sym <= 0x7f)) {
                key = event->keysym.sym;
            }
            break;
    }
    return key;
}

static void I_VideoDriver_PostEvent(const evtype_t type, const int data1, const int data2, const int data3)
{
    event_t* event = &events[eventhead];
    eventhead = ((eventhead + 1) & (MAXEVENTS - 1));
    event->type  = type;
    event->data1 = data1;
    event->data2 = data2;
    event->data3 = data3;
}

static void I_VideoDriver_OnQuit(I_VideoDriver* self, const SDL_QuitEvent* event)
{
    I_VideoDriver_PostEvent(ev_quit, 0, 0, 0);
}

static void I_VideoDriver_OnKeyDown(I_VideoDriver* self, const SDL_KeyboardEvent* event)
{
    const int key = I_VideoDriver_TranslateKey(self, event);

    if(key >= 0) {
        I_VideoDriver_PostEvent(ev_keydown, key, 0, 0);
    }
}

static void I_VideoDriver_OnKeyUp(I_VideoDriver* self, const SDL_KeyboardEvent* event)
{
    const int key = I_VideoDriver_TranslateKey(self, event);

    if(key >= 0) {
        I_VideoDriver_PostEvent(ev_keyup, key, 0, 0);
    }
}

static void I_VideoDriver_OnMouseMotion(I_VideoDriver* self, const SDL_MouseMotionEvent* event)
{
    /* update mouse position */ {
        self->mouse_x = event->x;
        self->mouse_y = event->y;
    }
}

static void I_VideoDriver_OnMouseButtonDown(I_VideoDriver* self, const SDL_MouseButtonEvent* event)
{
    /* update mouse buttons */ {
        switch(event->button) {
            case 1:
                self->mouse_b |= 1;
                break;
            case 2:
                self->mouse_b |= 2;
                break;
            case 3:
                self->mouse_b |= 4;
                break;
            default:
                break;
        }
    }
    /* update mouse position */ {
        self->mouse_x = event->x;
        self->mouse_y = event->y;
    }
    /* post mouse event */ {
        const int mouse_b = self->mouse_b;
        const int mouse_x = 0;
        const int mouse_y = 0;
        I_VideoDriver_PostEvent(ev_mouse, mouse_b, mouse_x, mouse_y);
    }
}

static void I_VideoDriver_OnMouseButtonUp(I_VideoDriver* self, const SDL_MouseButtonEvent* event)
{
    /* update mouse buttons */ {
        switch(event->button) {
            case 1:
                self->mouse_b &= ~1;
                break;
            case 2:
                self->mouse_b &= ~2;
                break;
            case 3:
                self->mouse_b &= ~4;
                break;
            default:
                break;
        }
    }
    /* update mouse position */ {
        self->mouse_x = event->x;
        self->mouse_y = event->y;
    }
    /* post mouse event */ {
        const int mouse_b = self->mouse_b;
        const int mouse_x = 0;
        const int mouse_y = 0;
        I_VideoDriver_PostEvent(ev_mouse, mouse_b, mouse_x, mouse_y);
    }
}

static void I_VideoDriver_OnWindowEvent(I_VideoDriver* self, const SDL_WindowEvent* event)
{
    switch(event->event) {
        case SDL_WINDOWEVENT_MOVED:
            self->window_x = event->data1;
            self->window_y = event->data2;
            break;
        case SDL_WINDOWEVENT_RESIZED:
            self->window_w = event->data1;
            self->window_h = event->data2;
            self->center_x = (self->window_w / 2);
            self->center_y = (self->window_h / 2);
            break;
        case SDL_WINDOWEVENT_ENTER:
            self->mouse_x = self->center_x;
            self->mouse_y = self->center_y;
            break;
        case SDL_WINDOWEVENT_LEAVE:
            self->mouse_x = self->center_x;
            self->mouse_y = self->center_y;
            break;
        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// I_VideoDriver video interface
// ---------------------------------------------------------------------------

void I_VideoDriver_Video_Init(I_VideoDriver* self)
{
    I_VideoDriver_Init(self);
}

void I_VideoDriver_Video_Fini(I_VideoDriver* self)
{
    I_VideoDriver_Fini(self);
}

void I_VideoDriver_Video_DoNothing(I_VideoDriver* self)
{
}

void I_VideoDriver_Video_StartFrame(I_VideoDriver* self)
{
}

void I_VideoDriver_Video_ProcessEvents(I_VideoDriver* self)
{
    /* poll and dispatch sdl events */ {
        SDL_Event event;
        while(SDL_PollEvent(&event) != 0) {
            switch(event.type) {
                case SDL_QUIT:
                    I_VideoDriver_OnQuit(self, &event.quit);
                    break;
                case SDL_KEYDOWN:
                    I_VideoDriver_OnKeyDown(self, &event.key);
                    break;
                case SDL_KEYUP:
                    I_VideoDriver_OnKeyUp(self, &event.key);
                    break;
                case SDL_MOUSEMOTION:
                    I_VideoDriver_OnMouseMotion(self, &event.motion);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    I_VideoDriver_OnMouseButtonDown(self, &event.button);
                    break;
                case SDL_MOUSEBUTTONUP:
                    I_VideoDriver_OnMouseButtonUp(self, &event.button);
                    break;
                case SDL_WINDOWEVENT:
                    I_VideoDriver_OnWindowEvent(self, &event.window);
                    break;
                default:
                    break;
            }
        }
    }
    /* post mouse event */ {
        const int div = 16;
        const int mul = 2;
        const int mouse_b = self->mouse_b;
        const int mouse_x = (((self->mouse_x - self->center_x) / div) * mul);
        const int mouse_y = (((self->center_y - self->mouse_y) / div) * mul);
        I_VideoDriver_PostEvent(ev_mouse, mouse_b, mouse_x, mouse_y);
    }
}

void I_VideoDriver_Video_RenderFrame(I_VideoDriver* self)
{
    /* update picture */ {
        SDL_BlitSurface(self->surface, NULL, self->picture, NULL);
    }
    /* update texture */ {
        Uint32 img_format = self->picture->format->format;
        int    img_width  = self->picture->w;
        int    img_height = self->picture->h;
        int    img_pitch  = self->picture->pitch;
        void*  img_pixels = self->picture->pixels;
        Uint32 tex_format = 0;
        int    tex_width  = 0;
        int    tex_height = 0;
        int    tex_pitch  = 0;
        void*  tex_pixels = NULL;
        SDL_QueryTexture(self->texture, &tex_format, NULL, &tex_width, &tex_height);
        SDL_LockTexture(self->texture, NULL, &tex_pixels, &tex_pitch);
        SDL_ConvertPixels(img_width, img_height, img_format, img_pixels, img_pitch, tex_format, tex_pixels, tex_pitch);
        SDL_UnlockTexture(self->texture);
    }
    /* blit to screen */ {
        SDL_RenderCopy(self->renderer, self->texture, NULL, NULL);
        SDL_RenderPresent(self->renderer);
    }
}

void I_VideoDriver_Video_ReadScreen(I_VideoDriver* self, byte* screen)
{
    (void) memcpy(screen, screens[0], (self->screen_w * self->screen_h));
}

void I_VideoDriver_Video_SetPalette(I_VideoDriver* self, byte* palette)
{
    const int count = self->palette->ncolors;
    for(int index = 0; index < count; ++index) {
        SDL_Color* color = &self->palette->colors[index];
        color->r = gammatable[usegamma][*palette++];
        color->g = gammatable[usegamma][*palette++];
        color->b = gammatable[usegamma][*palette++];
        color->a = 255;
    }
    if(++self->palette->version == 0) {
        self->palette->version = 1;
    }
}

// ---------------------------------------------------------------------------
// DOOM video interface
// ---------------------------------------------------------------------------

void I_InitGraphics(void)
{
    I_VideoDriver_Video_Init(&g_video_driver);
}

void I_ShutdownGraphics(void)
{
    I_VideoDriver_Video_Fini(&g_video_driver);
}

void I_StartFrame(void)
{
    I_VideoDriver_Video_StartFrame(&g_video_driver);
}

void I_StartTic(void)
{
    I_VideoDriver_Video_ProcessEvents(&g_video_driver);
}

void I_UpdateNoBlit(void)
{
    I_VideoDriver_Video_DoNothing(&g_video_driver);
}

void I_FinishUpdate(void)
{
    I_VideoDriver_Video_RenderFrame(&g_video_driver);
}

void I_ReadScreen(byte* screen)
{
    I_VideoDriver_Video_ReadScreen(&g_video_driver, screen);
}

void I_SetPalette(byte* palette)
{
    I_VideoDriver_Video_SetPalette(&g_video_driver, palette);
}

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------
