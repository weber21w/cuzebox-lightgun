/*
 *  Game input processing
 *
 *  Copyright (C) 2016 - 2018
 *    Sandor Zsuga (Jubatian)
 *  Uzem (the base of CUzeBox) is copyright (C)
 *    David Etherton,
 *    Eric Anderton,
 *    Alec Bourque (Uze),
 *    Filipe Rinaldi,
 *    Sandor Zsuga (Jubatian),
 *    Matt Pandina (Artcfox)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "ginput.h"
#include "cu_ctr.h"
#include "cu_kbd.h"
#include "cu_mouse.h"
#include "cu_gun.h"

extern auint guicore_getflags(void);
extern SDL_Window*   guicore_window;

/* Request Uzem style keymapping */
static boole ginput_kbuzem = FALSE;

/* Directing keyboard input to Player 2 (for two redirect keys) */
static boole ginput_kbp2_0 = FALSE;
static boole ginput_kbp2_1 = FALSE;

/* Request 2 players controller allocation */
static boole ginput_2palloc = FALSE;

#ifndef USE_SDL1
/* Game controller objects */
static SDL_GameController* ginput_gamectr[2] = {NULL, NULL};

/* Event ID for each game controller */
static auint ginput_gamectr_id[2] = {0U, 0U};

/* Directional moves collected from digital inputs */
static boole ginput_gamectr_ddig[2][4];

/* Directional moves collected from analog inputs */
static boole ginput_gamectr_dana[2][4];

#ifndef HEADLESS
/* Controller name when no name string is available */
static const char ginput_ctr_noname[] = "<no name>";
#endif
/* Game controller DB filename */
static const char ginput_gctr_filename[] = "gamecontrollerdb.txt";
#endif



#ifndef USE_SDL1

/*
** Returns controller name string for the given controller. Returns no-name
** string if it is nonexistent.
*/
#ifndef HEADLESS
static const char* ginput_gctr_name(auint i)
{
 const char* tstr;
 if (i >= 2U){ return &(ginput_ctr_noname[0]); }
 if (ginput_gamectr[i] == NULL){ return &(ginput_ctr_noname[0]); }
 tstr = SDL_GameControllerName(ginput_gamectr[i]);
 if (tstr == NULL){ return &(ginput_ctr_noname[0]); }
 return tstr;
}
#endif

/*
** Returns player number (0 or 1) for which the game controller event should
** apply. If there is one controller, it is for Player 2 when 2 player mode is
** enabled.
*/
static auint ginput_gctr_getplayer(SDL_Event const* ev)
{
 SDL_JoystickID jid;
 if (!ginput_2palloc){ return 0U; }
 if (ginput_gamectr[1] == NULL){ return 1U; }
 if (ev->type == SDL_JOYAXISMOTION){ jid = ev->jaxis.which; }
 else                              { jid = ev->cbutton.which; }
 if ( jid != (ginput_gamectr_id[0]) &&
      jid == (ginput_gamectr_id[1]) ){ return 1U; }
 return 0U;
}


/*
** Prints out controller identification for players
*/
static void ginput_gctr_printid(auint player, auint cid)
{
 if (ginput_gamectr[cid] != NULL){
  print_message("Player %u controller id: %u name: %s\n",
      player,
      ginput_gamectr_id[cid],
      ginput_gctr_name(cid));
 }
}


/*
** Prints out player - controller allocation
*/
static void ginput_gctr_printalloc(void)
{
 if (ginput_2palloc){
  if (ginput_gamectr[1] == NULL){
   ginput_gctr_printid(2U, 0U); /* Only controller is for Player 2 */
  }else{
   ginput_gctr_printid(1U, 0U); /* Controller 0 is for Player 1 */
   ginput_gctr_printid(2U, 1U); /* Controller 1 is for Player 2 */
  }
 }else{
  ginput_gctr_printid(1U, 0U); /* Controller 0 is for Player 1 */
  ginput_gctr_printid(1U, 1U); /* Controller 1 is for Player 1 */
 }
}


/*
** Loads game controller config. from path along with reporting it
*/
static void ginput_gctr_loadmappings(char const* path)
{
 asint n = SDL_GameControllerAddMappingsFromFile(path);
 print_message("Load game controller mappings:\n    %s\n    ", path);
 if (n < 0){ print_unf("could not open\n"); }
 else{       print_message("contained %i entries\n", n); }
}

#endif



/*
** Initializes input component. Always succeeds (it may just fail to find any
** input device, the emulator however might still run a demo not needing any).
*/
void  ginput_init(void)
{
#ifdef HEADLESS
 return;
#endif
#ifndef USE_SDL1
 auint         i;
 auint         j;
 SDL_Joystick* jtmp;
 char          sbuf[1024];
 char*         bpat;
#endif

#ifndef USE_SDL1

 for (j = 0U; j < 2U; j++){
  for (i = 0U; i < 4U; i++){
   ginput_gamectr_ddig[j][i] = FALSE;
   ginput_gamectr_dana[j][i] = FALSE;
  }
 }

 /* Load game controller mappings (if such a file is present) */

 bpat = SDL_GetBasePath();
 i = 0U;
 if (bpat != NULL){
  while (bpat[i] != 0){
   sbuf[i] = bpat[i];
   i++;
   if (i == 1023U){ break; }
  }
  SDL_free(bpat);
 }
 j = 0U;
 while (ginput_gctr_filename[j] != 0){
  if (i == 1023U){ break; }
  sbuf[i] = ginput_gctr_filename[j];
  i++;
  j++;
 }
 sbuf[i] = 0;
 ginput_gctr_loadmappings(&(sbuf[0]));
#ifdef PATH_GAMECONTROLLERDB
 ginput_gctr_loadmappings(PATH_GAMECONTROLLERDB);
#endif

 /* Open game controller or controllers */

 j = 0U;
 for (i = 0U; i < SDL_NumJoysticks(); i++){
  if (SDL_IsGameController(i)){
   ginput_gamectr[j] = SDL_GameControllerOpen(i);
   if (ginput_gamectr[j] != NULL){
    jtmp = SDL_GameControllerGetJoystick(ginput_gamectr[j]);
    if (jtmp != NULL){
     ginput_gamectr_id[j] = SDL_JoystickInstanceID(jtmp);
     print_message("Game controller id: %u found: %s\n",
         ginput_gamectr_id[j],
         ginput_gctr_name(j));
     j++;
     if (j >= 2U){ break; }
    }else{
     SDL_GameControllerClose(ginput_gamectr[j]);
     ginput_gamectr[j] = NULL;
    }
   }
  }
 }

 /* If there are 2 game controllers, start in 2 player mode, otherwise assume
 ** 1 player initially. */

 ginput_2palloc = (j >= 2U);
 ginput_gctr_printalloc();

#endif
}



/*
** Frees / destroys input component.
*/
void  ginput_quit(void)
{
#ifndef USE_SDL1
 auint i;
#endif

#ifndef USE_SDL1
 /* Close game controller or controllers */

 for (i = 0U; i < 2U; i++){
  if (ginput_gamectr[i] != NULL){
   SDL_GameControllerClose(ginput_gamectr[i]);
  }
 }
#endif
}



/*
** Forwards SDL events to emulated game controllers.
*/
void  ginput_sendevent(SDL_Event const* ev)
{

 boole press = FALSE;
 auint player = 0U;

 /* Keyboard input: Player 1 */
 if(cu_kbd_get_enabled() && ((ev->type) == SDL_KEYDOWN || (ev->type) == SDL_KEYUP)){
  cu_kbd_handle_key(ev);
 }else if ( (((ev->type) == SDL_KEYDOWN) ||
      ((ev->type) == SDL_KEYUP))){ /* skip key interpretation if keyboard capture is enabled */
 
 if ((ev->type) == SDL_KEYDOWN){ press = TRUE; }
  if ( (ginput_kbp2_0) ||
       (ginput_kbp2_1) ){ player = 1U; }

  /* Note: For SDL2 the scancode has more sense here, but SDL1 does not
  ** support that. This solution works for now on both. For the Uzem
  ** keymapping both Y and Z triggers SNES_Y, so it remains useful on both a
  ** QWERTY and a QWERTZ keyboard. */

  switch (ev->key.keysym.sym){
   case SDLK_RALT:
    ginput_kbp2_0 = press; /* AltGr down => Keyboard input goes to P2 */
    break;
   case SDLK_LALT:
    ginput_kbp2_1 = press; /* Alt down => Keyboard input goes to P2 */
    break;
   case SDLK_LEFT:
#ifndef USE_SDL1
    ginput_gamectr_ddig[player][0] = press;
#else
    cu_ctr_setsnes_single(player, CU_CTR_SNES_LEFT, press);
#endif
    break;
   case SDLK_RIGHT:
#ifndef USE_SDL1
    ginput_gamectr_ddig[player][1] = press;
#else
    cu_ctr_setsnes_single(player, CU_CTR_SNES_RIGHT, press);
#endif
    break;
   case SDLK_UP:
#ifndef USE_SDL1
    ginput_gamectr_ddig[player][2] = press;
#else
    cu_ctr_setsnes_single(player, CU_CTR_SNES_UP, press);
#endif
    break;
   case SDLK_DOWN:
#ifndef USE_SDL1
    ginput_gamectr_ddig[player][3] = press;
#else
    cu_ctr_setsnes_single(player, CU_CTR_SNES_DOWN, press);
#endif
    break;
   case SDLK_q:
    if (!ginput_kbuzem){ cu_ctr_setsnes_single(player, CU_CTR_SNES_Y, press); }
    break;
   case SDLK_w:
    if (!ginput_kbuzem){ cu_ctr_setsnes_single(player, CU_CTR_SNES_X, press); }
    break;
   case SDLK_a:
    if (!ginput_kbuzem){ cu_ctr_setsnes_single(player, CU_CTR_SNES_B, press); }
    if ( ginput_kbuzem){ cu_ctr_setsnes_single(player, CU_CTR_SNES_A, press); }
    break;
   case SDLK_s:
    if (!ginput_kbuzem){ cu_ctr_setsnes_single(player, CU_CTR_SNES_A, press); }
    if ( ginput_kbuzem){ cu_ctr_setsnes_single(player, CU_CTR_SNES_B, press); }
    break;
   case SDLK_y:
    if ( ginput_kbuzem){ cu_ctr_setsnes_single(player, CU_CTR_SNES_Y, press); }
    break;
   case SDLK_z:
    if ( ginput_kbuzem){ cu_ctr_setsnes_single(player, CU_CTR_SNES_Y, press); }
    break;
   case SDLK_x:
    if ( ginput_kbuzem){ cu_ctr_setsnes_single(player, CU_CTR_SNES_X, press); }
    break;
   case SDLK_SPACE:
    cu_ctr_setsnes_single(player, CU_CTR_SNES_SELECT, press);
    break;
   case SDLK_TAB:
    cu_ctr_setsnes_single(player, CU_CTR_SNES_SELECT, press);
    break;
   case SDLK_RETURN:
    cu_ctr_setsnes_single(player, CU_CTR_SNES_START, press);
    break;
   case SDLK_LSHIFT:
    cu_ctr_setsnes_single(player, CU_CTR_SNES_LSH, press);
    break;
   case SDLK_RSHIFT:
    cu_ctr_setsnes_single(player, CU_CTR_SNES_RSH, press);
    break;
   default:
    break;
  }

 }

#ifndef USE_SDL1

 /* Controller input */

 if ( ((ev->type) == SDL_CONTROLLERBUTTONDOWN) ||
      ((ev->type) == SDL_CONTROLLERBUTTONUP)){

  player = ginput_gctr_getplayer(ev);
  if(!cu_kbd_get_enabled() || player != 1){ /* ignore P2 input if the keyboard dongle is enabled */

   if ((ev->type) == SDL_CONTROLLERBUTTONDOWN){ press = TRUE; }

   switch (ev->cbutton.button){
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
     ginput_gamectr_ddig[player][0] = press;
     break;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
     ginput_gamectr_ddig[player][1] = press;
     break;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
     ginput_gamectr_ddig[player][2] = press;
     break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
     ginput_gamectr_ddig[player][3] = press;
     break;
    case SDL_CONTROLLER_BUTTON_Y: /* X-Y swapped due to SDL2 layout corresponding to XBox360 */
     cu_ctr_setsnes_single(player, CU_CTR_SNES_X, press);
     break;
    case SDL_CONTROLLER_BUTTON_X: /* X-Y swapped due to SDL2 layout corresponding to XBox360 */
     cu_ctr_setsnes_single(player, CU_CTR_SNES_Y, press);
     break;
    case SDL_CONTROLLER_BUTTON_B: /* A-B swapped due to SDL2 layout corresponding to XBox360 */
     cu_ctr_setsnes_single(player, CU_CTR_SNES_A, press);
     break;
    case SDL_CONTROLLER_BUTTON_A: /* A-B swapped due to SDL2 layout corresponding to XBox360 */
     cu_ctr_setsnes_single(player, CU_CTR_SNES_B, press);
     break;
    case SDL_CONTROLLER_BUTTON_BACK:
     cu_ctr_setsnes_single(player, CU_CTR_SNES_SELECT, press);
     break;
    case SDL_CONTROLLER_BUTTON_GUIDE:
     cu_ctr_setsnes_single(player, CU_CTR_SNES_SELECT, press);
     break;
    case SDL_CONTROLLER_BUTTON_START:
     cu_ctr_setsnes_single(player, CU_CTR_SNES_START, press);
     break;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
     cu_ctr_setsnes_single(player, CU_CTR_SNES_LSH, press);
     break;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
     cu_ctr_setsnes_single(player, CU_CTR_SNES_RSH, press);
     break;
    default:
     break;
   }
  }
 }

 /* Joystick axis input. This complements controller input, necessary due to
 ** the poor API not being able to handle game controllers properly. If SDL2
 ** will be fixed at some point in the future, this may be removed (along with
 ** related patch in ginput_gctr_getplayer()). */

 if ( ((ev->type) == SDL_JOYAXISMOTION) ){ /* ignore P2 input if the keyboard dongle is enabled */

  player = ginput_gctr_getplayer(ev);
  if(!cu_kbd_get_enabled() || player != 1){
   if ((ev->jaxis.axis) == 0U){ /* X axis: Left and Right */
    press = ((ev->jaxis.value) <= -16384);
    ginput_gamectr_dana[player][0] = press;
    press = ((ev->jaxis.value) >=  16384);
    ginput_gamectr_dana[player][1] = press;
   }

   if ((ev->jaxis.axis) == 1U){ /* Y axis: Up and Down */
    press = ((ev->jaxis.value) <= -16384);
    ginput_gamectr_dana[player][2] = press;
    press = ((ev->jaxis.value) >=  16384);
    ginput_gamectr_dana[player][3] = press;
   }
  }
 }

 /* Mouse axis input. Data grabbed here will be built/encoded when controller data is latched, see cu_ctr.c, cu_mouse.c, and cu_gun.c */
 if ((ev->type) == SDL_MOUSEMOTION){
  if (cu_mouse_get_enabled()){
   sint32 mdx, mdy;
   if (cu_mouse_get_enabled()){
    cu_mouse_accumulate_buttons(SDL_GetRelativeMouseState(&mdx,&mdy)); /* buttons pressed and delta movements are accumulated since last Latch/reset */
    cu_mouse_accumulate_deltas(mdx,mdy);

    //if (guicore_getflags() & 0x0001U){ /* GUICORE_FULLSCREEN HACK TODO */
    // SDL_WarpMouseInWindow(guicore_window,400,300); /* keep mouse centered so it doesn't get stuck on the edge of the screen... */
    // SDL_GetRelativeMouseState(&mdx,&mdy); /*...and immediately consume the bogus motion event it generated. */
    //}
   }else if(cu_gun_get_enabled(0)){ /* Lightgun control */
    //cu_gun_set_trigger(SDL_GetMouseState(&mdx,&mdy),0U); /* relative to top left of window, returns mouse buttons, and writes x/y to mdx/mdy */
    //cu_gun_set_position(mdx,mdy,0U);
    
   }
  }
 }

 for (player = 0U; player < 2U; player ++){
  if (player == 0U && cu_mouse_get_enabled()){ continue; }
  if (player == 1U && cu_kbd_get_enabled()){ continue; }
  cu_ctr_setsnes_single(player, CU_CTR_SNES_LEFT,  ginput_gamectr_ddig[player][0] || ginput_gamectr_dana[player][0]);
  cu_ctr_setsnes_single(player, CU_CTR_SNES_RIGHT, ginput_gamectr_ddig[player][1] || ginput_gamectr_dana[player][1]);
  cu_ctr_setsnes_single(player, CU_CTR_SNES_UP,    ginput_gamectr_ddig[player][2] || ginput_gamectr_dana[player][2]);
  cu_ctr_setsnes_single(player, CU_CTR_SNES_DOWN,  ginput_gamectr_ddig[player][3] || ginput_gamectr_dana[player][3]);
 }

#endif

}


/*
** Sets SNES / UZEM style keymapping (TRUE: UZEM)
*/
void  ginput_setkbuzem(boole kmap)
{
 ginput_kbuzem = kmap;
}



/*
** Retrieves SNES / UZEM style keymapping select (TRUE: UZEM)
*/
boole ginput_iskbuzem(void)
{
 return ginput_kbuzem;
}



/*
** Sets 1 player / 2 players controller allocation (TRUE: 2 players)
*/
void  ginput_set2palloc(boole al2p)
{
#ifndef USE_SDL1
 if (al2p != ginput_2palloc){
  ginput_2palloc = al2p;
  ginput_gctr_printalloc();
 }
#else
 ginput_2palloc = al2p;
#endif
}



/*
** Retrieves 1 player / 2 players controller allocation (TRUE: 2 players)
*/
boole ginput_is2palloc(void)
{
 return ginput_2palloc;
}
