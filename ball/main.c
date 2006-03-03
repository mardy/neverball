/*   
 * Copyright (C) 2003 Robert Kooima
 *
 * NEVERBALL is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 2  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

/*---------------------------------------------------------------------------*/

#ifdef WIN32
#pragma comment(lib, "SDL_ttf.lib")
#pragma comment(lib, "SDL_mixer.lib")
#pragma comment(lib, "SDL_image.lib")
#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")
#pragma comment(lib, "opengl32.lib")
#endif

/*---------------------------------------------------------------------------*/

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "glext.h"
#include "config.h"
#include "image.h"
#include "audio.h"
#include "demo.h"
#include "levels.h"
#include "game.h"
#include "gui.h"
#include "set.h"

#include "st_conf.h"
#include "st_title.h"
#include "st_demo.h"
#include "st_level.h"

#define TITLE "Neverball"
#define VERSION "1.4.1svn"

/*---------------------------------------------------------------------------*/

static void shot(void)
{
    static char filename[MAXSTR];
    static int  num = 0;

    sprintf(filename, _("screen%02d.bmp"), num++);

    image_snap(filename, config_get_d(CONFIG_WIDTH), config_get_d(CONFIG_HEIGHT));
}

/*---------------------------------------------------------------------------*/

static void toggle_wire(void)
{
    static int wire = 0;

    if (wire)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
        wire = 0;
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        wire = 1;
    }
}

static void toggle_fullscreen(void)
{
    int x, y;
    SDL_GetMouseState(&x, &y);
    config_mode(!config_get_d(CONFIG_FULLSCREEN), config_get_d(CONFIG_WIDTH), config_get_d(CONFIG_HEIGHT));
    SDL_WarpMouse(x, y);
}


/*---------------------------------------------------------------------------*/

static int loop(void)
{
    SDL_Event e;
    int d = 1;

    while (d && SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            return 0;

        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE)
            config_tgl_pause();

        if (!config_get_pause())
            switch (e.type)
            {
            case SDL_MOUSEMOTION:
                st_point(+e.motion.x,
                         -e.motion.y + config_get_d(CONFIG_HEIGHT),
                         +e.motion.xrel,
                         config_get_d(CONFIG_MOUSE_INVERT)
                         ? +e.motion.yrel : -e.motion.yrel);
                break;

            case SDL_MOUSEBUTTONDOWN:
                d = st_click((e.button.button == SDL_BUTTON_LEFT) ? -1 : 1, 1);
                break;
                
            case SDL_MOUSEBUTTONUP:
                d = st_click((e.button.button == SDL_BUTTON_LEFT) ? -1 : 1, 0);
                break;

            case SDL_KEYDOWN:
                
                switch (e.key.keysym.sym)
                {
                case SDLK_F11:   toggle_fullscreen();       break;
                case SDLK_F10:   shot();                    break;
                case SDLK_F9:    config_tgl_d(CONFIG_FPS);  break;
                case SDLK_F8:    config_tgl_d(CONFIG_NICE); break;
                case SDLK_F7:    toggle_wire();             break;
                
                case SDLK_RETURN:
                    d = st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_A), 1);
                    break;
                case SDLK_ESCAPE:
                    d = st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_EXIT), 1);
                    break;
                case SDLK_LEFT:
                    st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_X), -JOY_MAX);
                    break;
                case SDLK_RIGHT:
                    st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_X), +JOY_MAX);
                    break;
                case SDLK_UP:
                    st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_Y), -JOY_MAX);
                    break;
                case SDLK_DOWN:
                    st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_Y), +JOY_MAX);
                    break;
                             
                default:
		    if (SDL_EnableUNICODE(-1)) 
                        d = st_keybd(e.key.keysym.unicode, 1);
		    else
                        d = st_keybd(e.key.keysym.sym, 1);
                }
                break;

            case SDL_KEYUP:

                switch (e.key.keysym.sym)
                {
                case SDLK_RETURN:
                    d = st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_A), 0);
                    break;
                case SDLK_ESCAPE:
                    d = st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_EXIT), 0);
                    break;
                case SDLK_LEFT:
                case SDLK_RIGHT:
                    st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_X), 1);
                    break;
                case SDLK_DOWN:
                case SDLK_UP:
                    st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_Y), 1);
                    break;

                default:
                    d = st_keybd(e.key.keysym.sym, 0);
                }

                break;

            case SDL_ACTIVEEVENT:
                if (e.active.state == SDL_APPINPUTFOCUS)
                    if (e.active.gain == 0 && config_get_grab())
                        config_set_pause();
                break;

            case SDL_JOYAXISMOTION:
                st_stick(e.jaxis.axis, e.jaxis.value);
                break;

            case SDL_JOYBUTTONDOWN:
                d = st_buttn(e.jbutton.button, 1);
                break;

            case SDL_JOYBUTTONUP:
                d = st_buttn(e.jbutton.button, 0);
                break;
            }
    }
    return d;
}

/*---------------------------------------------------------------------------*/

/* Option values */
static char * data_path   = NULL;
static char * replay_path = NULL;
static char * level_path  = NULL;

/* Option hangling */

#define USAGE  _( \
	"Usage: %s [options ...]\n" \
	"-r, --replay file         play the replay 'file'.\n" \
	"-l, --level file.sol      play the level 'file.sol'.\n" \
	"    --data dir            use 'dir' as game data directory.\n" \
	"-v, --version             show version.\n" \
	"-h, -?, --help            show this usage message.\n")

static void parse_args(int argc, char ** argv)
{
#define CASE(x) (strcmp(*argv, (x)) == 0)        /* Check current option */
#define MAND    (not_miss = (argv[1] != NULL)) /* Argument is mandatory */
    char * exec = *(argv++);
    int not_miss; /* argument is not missing */
    
    while (*argv != NULL)
    {
	not_miss = 1;
	if (CASE("-h") || CASE("-?") || CASE("--help"))
	{
	    printf(USAGE, exec);
	    exit(0);
	}
	else if (CASE("-v") || CASE("--version"))
	{
	    printf(_("%s: %s version %s\n"), exec, TITLE, VERSION);
	    exit(0);
	}
	else if (CASE("--data") && MAND)
	    data_path = *(++argv);
	else if ((CASE("-r") || CASE("--replay")) && MAND)
	    replay_path = *(++argv);
	else if ((CASE("-l")  || CASE("--level")) && MAND)
	    level_path = *(++argv);
	else if (not_miss)
	{
	    fprintf(stderr, _("%s: unknown option %s\n"), exec, *argv);
	    fprintf(stderr, USAGE, exec);
	    exit(1);
	}
	else
	{
	    fprintf(stderr, _("%s: option %s requires an argument\n"), exec, *argv);
	    fprintf(stderr, USAGE, exec);
	    exit(1);
	}
	argv++;
    }
    return;
}

int main(int argc, char *argv[])
{
    SDL_Joystick *joy = NULL;
    int t1, t0;               /* ticks */
    SDL_Surface *icon;        /* WM icon */
   
    language_init("neverball", CONFIG_LOCALE);

    parse_args(argc, argv);
    
    if (!config_data_path(data_path, SET_FILE))
    {
        fprintf(stderr, _("Failure to establish game data directory\n"));
	return 1;
    }
    
    if (!config_user_path(NULL))
    {
        fprintf(stderr, _("Failure to establish config directory\n"));
	return 1;
    }
    
    /* Intitialize the configuration */
    
    config_init();
    config_load();
    
    /* Initialize the language */
    
    language_set(language_from_code(config_simple_get_s(CONFIG_LANG)));

    /* Prepare run without sdl */
    
    if (replay_path != NULL)
    {
	if (level_replay(replay_path))
	{
	    demo_replay_dump_info();
	}
	else
	{
	    fprintf(stderr, _("Replay file '%s': "), replay_path);
	    if (errno)
		perror(NULL);
	    else
		fprintf(stderr, _("Not a replay file.\n"));
	    return 1;
	}
    }
    
    /* Initialize SDL system and subsystems */
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) == -1)
    {
	fprintf(stderr, "%s\n", SDL_GetError());
	return 1;
    }

    /* Initialize the joystick. */

    if (SDL_NumJoysticks() > 0)
    {
	joy=SDL_JoystickOpen(config_get_d(CONFIG_JOYSTICK_DEVICE));
	if (joy)
		SDL_JoystickEventState(SDL_ENABLE);
    }

    /* Initialize the audio. */

    audio_bind(AUD_MENU,   3, "snd/menu.wav");
    audio_bind(AUD_START,  1, "snd/select.ogg");
    audio_bind(AUD_READY,  1, "snd/ready.ogg");
    audio_bind(AUD_SET,    1, "snd/set.ogg");
    audio_bind(AUD_GO,     1, "snd/go.ogg");
    audio_bind(AUD_BALL,   2, "snd/ball.ogg");
    audio_bind(AUD_BUMP,   3, "snd/bump.ogg");
    audio_bind(AUD_COIN,   2, "snd/coin.wav");
    audio_bind(AUD_TICK,   4, "snd/tick.ogg");
    audio_bind(AUD_TOCK,   4, "snd/tock.ogg");
    audio_bind(AUD_SWITCH, 5, "snd/switch.wav");
    audio_bind(AUD_JUMP,   5, "snd/jump.ogg");
    audio_bind(AUD_GOAL,   5, "snd/goal.wav");
    audio_bind(AUD_SCORE,  1, "snd/record.ogg");
    audio_bind(AUD_FALL,   1, "snd/fall.ogg");
    audio_bind(AUD_TIME,   1, "snd/time.ogg");
    audio_bind(AUD_OVER,   1, "snd/over.ogg");

    audio_init();

    /* Require 16-bit double buffer with 16-bit depth buffer. */

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    /* Initialize the video. */

    if (! config_mode(config_get_d(CONFIG_FULLSCREEN),
	            config_get_d(CONFIG_WIDTH),
		    config_get_d(CONFIG_HEIGHT)))
    {
	fprintf(stderr, "%s\n", SDL_GetError());
	return 1;
    }
   
    /* Set the WM icon */ 
    
    icon = IMG_Load(config_data("icon/neverball.png"));
    SDL_WM_SetIcon(icon, NULL);
    SDL_WM_SetCaption(TITLE, TITLE); 

    /* Initialize the run state. */
    
    init_state(&st_null);
    if (replay_path != NULL)
    {
	level_replay(replay_path);
	goto_demo_play(1);
    }
    else if (level_path != NULL)
    {
	level_play_single(level_path);
	goto_state(&st_level);
    }
    else
	goto_state(&st_title);

    /* Run the main game loop. */

    t0 = SDL_GetTicks();
    while (loop())
	if ((t1 = SDL_GetTicks()) > t0)
	{
	    if (config_get_pause())
	    {
		st_paint();
		gui_blank();
	    }
	    else
	    {
		st_timer((t1 - t0) / 1000.f);
		st_paint();
	    }
	    SDL_GL_SwapBuffers();

	    t0 = t1;

	    if (config_get_d(CONFIG_NICE))
		SDL_Delay(1);
	}

    /* Gracefully close the game */

    if (SDL_JoystickOpened(0))
	SDL_JoystickClose(joy);

    SDL_Quit();
    
    config_save();
    
    return 0;
}

/*---------------------------------------------------------------------------*/

