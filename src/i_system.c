// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include "doomdef.h"
#include "m_argv.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"
#include "d_net.h"
#include "g_game.h"
#ifdef __GNUG__
#pragma implementation "i_system.h"
#endif
#include "i_system.h"

int g_verbose    = VERBOSE_DEFAULT;
int g_scale_mul  = -1;
int g_scale_div  = -1;
int g_scale_mode = -1;
int g_quit_game  = 0;
int g_mb_used    = 6;

void I_ParseCommandLine (void)
{
    /* parse command-line for verbose level */ {
        int argi = 0;
        int last = 0;
        /* verbose level: quiet */ {
            if((argi = M_CheckParm("-quiet")) && (argi > last)) {
                g_verbose = VERBOSE_QUIET;
            }
        }
        /* verbose level: error */ {
            if((argi = M_CheckParm("-error")) && (argi > last)) {
                g_verbose = VERBOSE_ERROR;
            }
        }
        /* verbose level: alert */ {
            if((argi = M_CheckParm("-alert")) && (argi > last)) {
                g_verbose = VERBOSE_ALERT;
            }
        }
        /* verbose level: print */ {
            if((argi = M_CheckParm("-print")) && (argi > last)) {
                g_verbose = VERBOSE_PRINT;
            }
        }
        /* verbose level: debug */ {
            if((argi = M_CheckParm("-debug")) && (argi > last)) {
                g_verbose = VERBOSE_DEBUG;
            }
        }
        /* verbose level: trace */ {
            if((argi = M_CheckParm("-trace")) && (argi > last)) {
                g_verbose = VERBOSE_TRACE;
            }
        }
        /* verbose level: sanity checks */ {
            switch(g_verbose) {
                case VERBOSE_QUIET:
                case VERBOSE_ERROR:
                case VERBOSE_ALERT:
                case VERBOSE_PRINT:
                case VERBOSE_DEBUG:
                case VERBOSE_TRACE:
                    break;
                default:
                    g_verbose = VERBOSE_DEFAULT;
                    break;
            }
        }
    }
}

void I_Tactile (int on, int off, int total)
{
  // UNUSED.
  on = off = total = 0;
}

ticcmd_t emptycmd;

ticcmd_t* I_BaseTiccmd(void)
{
    return &emptycmd;
}


int I_GetHeapSize (void)
{
    return g_mb_used * 1024 * 1024;
}

byte* I_ZoneBase (int* size)
{
    *size = g_mb_used * 1024 * 1024;
    return (byte *) malloc (*size);
}



//
// I_GetTime
// returns time in 1/70th second tics
//
int I_GetTime (void)
{
    static int basetime = 0;
    struct timeval tp;
    int newtics;

    gettimeofday(&tp, NULL);
    if (!basetime)
        basetime = tp.tv_sec;
    newtics = (tp.tv_sec-basetime)*TICRATE + tp.tv_usec*TICRATE/1000000;
    return newtics;
}



//
// I_Init
//
void I_Init (void)
{
    I_InitSound();
    I_InitMusic();
//  I_InitGraphics();
}

//
// I_Quit
//
void I_Quit (void)
{
    if (demorecording)
        G_CheckDemoStatus();

    D_QuitNetGame ();
    M_SaveDefaults ();
    I_ShutdownSound ();
    I_ShutdownMusic ();
    I_ShutdownGraphics ();
#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
    emscripten_force_exit(EXIT_SUCCESS);
#else
    exit(EXIT_SUCCESS);
#endif
}

//
// I_Panic
//
void I_Panic (void)
{
    if (demorecording)
        G_CheckDemoStatus();

    D_QuitNetGame ();
    I_ShutdownSound ();
    I_ShutdownMusic ();
    I_ShutdownGraphics ();
#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
    emscripten_force_exit(EXIT_FAILURE);
#else
    exit(EXIT_FAILURE);
#endif
}

//
// I_Abort
//
void I_Abort (void)
{
#ifdef __EMSCRIPTEN__
    emscripten_force_exit(EXIT_FAILURE);
#else
    abort();
#endif
}

void I_WaitVBL(int count)
{
#ifdef SGI
    sginap(1);                                           
#else
#ifdef SUN
    sleep(0);
#else
    usleep((count * 1000000) / 70);                                
#endif
#endif
}

void I_BeginRead(void)
{
}

void I_EndRead(void)
{
}

byte* I_AllocLow(int length)
{
    byte* mem;

    mem = (byte *) malloc (length);
    memset (mem, 0, length);
    return mem;
}


//
// I_Fatal
//

void I_Fatal (const char *fatal, ...)
{
    va_list argptr;

    if(g_verbose >= VERBOSE_QUIET) {
        va_start (argptr, fatal);
        fputc ('F', stderr);
        fputc ('\t', stderr);
        vfprintf (stderr, fatal, argptr);
        fputc ('\n', stderr);
        va_end (argptr);
        fflush( stderr );
    }

    I_Abort ();
}

//
// I_Error
//

void I_Error (const char *error, ...)
{
    va_list argptr;

    if(g_verbose >= VERBOSE_ERROR) {
        va_start (argptr, error);
        fputc ('E', stderr);
        fputc ('\t', stderr);
        vfprintf (stderr, error, argptr);
        fputc ('\n', stderr);
        va_end (argptr);
        fflush( stderr );
    }

    I_Panic ();
}

//
// I_Alert
//

void I_Alert (const char *alert, ...)
{
    va_list argptr;

    if(g_verbose >= VERBOSE_ALERT) {
        va_start (argptr, alert);
        fputc ('W', stderr);
        fputc ('\t', stderr);
        vfprintf (stderr, alert, argptr);
        fputc ('\n', stderr);
        va_end (argptr);
        fflush( stderr );
    }
}

//
// I_Print
//

void I_Print (const char *print, ...)
{
    va_list argptr;

    if(g_verbose >= VERBOSE_PRINT) {
        va_start (argptr, print);
        fputc ('I', stderr);
        fputc ('\t', stderr);
        vfprintf (stderr, print, argptr);
        fputc ('\n', stderr);
        va_end (argptr);
        fflush( stderr );
    }
}

//
// I_Debug
//

void I_Debug (const char *debug, ...)
{
    va_list argptr;

    if(g_verbose >= VERBOSE_DEBUG) {
        va_start (argptr, debug);
        fputc ('D', stderr);
        fputc ('\t', stderr);
        vfprintf (stderr, debug, argptr);
        fputc ('\n', stderr);
        va_end (argptr);
        fflush( stderr );
    }
}

//
// I_Trace
//

void I_Trace (const char *trace, ...)
{
    va_list argptr;

    if(g_verbose >= VERBOSE_TRACE) {
        va_start (argptr, trace);
        fputc ('T', stderr);
        fputc ('\t', stderr);
        vfprintf (stderr, trace, argptr);
        fputc ('\n', stderr);
        va_end (argptr);
        fflush( stderr );
    }
}
