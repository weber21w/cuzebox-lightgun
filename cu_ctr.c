/*
 *  Controller emulation
 *
 *  Copyright (C) 2016
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



#include "cu_ctr.h"
#include "cu_kbd.h"
#include "cu_mouse.h"
#include "cu_gun.h"

/* SNES controller buttons */
#define CU_CTR_SNES_B        0U
#define CU_CTR_SNES_Y        1U
#define CU_CTR_SNES_SELECT   2U
#define CU_CTR_SNES_START    3U
#define CU_CTR_SNES_UP       4U
#define CU_CTR_SNES_DOWN     5U
#define CU_CTR_SNES_LEFT     6U
#define CU_CTR_SNES_RIGHT    7U
#define CU_CTR_SNES_A        8U
#define CU_CTR_SNES_X        9U
#define CU_CTR_SNES_LSH     10U
#define CU_CTR_SNES_RSH     11U


/* Current button state */
static auint cu_ctr_buttons[2];

/* Latched button state */
static auint cu_ctr_blatch[2];



/*
** Filters out mechanically impossible combinations in a button set
*/
static auint cu_ctr_mfilt(auint buttons)
{
 if ((buttons & (CU_CTR_SNES_M_UP | CU_CTR_SNES_M_DOWN)) == 0U){
  buttons |= CU_CTR_SNES_M_DOWN;
 }
 if ((buttons & (CU_CTR_SNES_M_LEFT | CU_CTR_SNES_M_RIGHT)) == 0U){
  buttons |= CU_CTR_SNES_M_RIGHT;
 }
 return buttons;
}



/*
** Resets controllers to all inactive.
*/
void  cu_ctr_reset(void)
{
 /* Controller buttons are low active */
 cu_ctr_buttons[0] = 0xFFFFFFFFU;
 cu_ctr_buttons[1] = 0xFFFFFFFFU;
 cu_ctr_blatch[0] = 0xFFFFFFFFU;
 cu_ctr_blatch[1] = 0xFFFFFFFFU;
}

/*
** Prepare controller state for subsequent serial clocks
*/
void cu_ctr_latch_event(void)
{
 /* Port 1 */
 cu_ctr_blatch[0] = cu_ctr_mfilt(cu_ctr_buttons[0]);//if mouse or lightgun is active, each device can cooperate with(or contain) a joypad

 if(cu_mouse_get_enabled()){
  cu_ctr_blatch[0] |= CU_MOUSE_SIGNATURE;
  cu_ctr_blatch[0] ^= CU_MOUSE_SIGNATURE;
  cu_mouse_latch_event();/* build buttons and delta report */
  cu_mouse_get_buttons();/* get button and delta data */
  //cu_ctr_blatch[0] ^= (cu_mouse_get_buttons() | CU_MOUSE_SIGNATURE);
 }else if(cu_gun_get_enabled(0)){
  cu_gun_latch_event();
  cu_ctr_blatch[0] |= CU_GUN_TRIGGER|CU_GUN_LIGHT;
  cu_ctr_blatch[0] ^= ((cu_gun_get_trigger(0)?CU_GUN_TRIGGER:0) | (cu_gun_get_light(0)?CU_GUN_LIGHT:0));
 }//else
  //cu_ctr_blatch[0] = cu_mouse_get_buttons(); /* if mouse enabled, the first 16 bits contain the button state and signature */





 /* Port 2 */
 if(!cu_kbd_get_enabled())
  cu_ctr_blatch[1] = cu_ctr_mfilt(cu_ctr_buttons[1]);
 //else
  //cu_ctr_blatch[1] = cu_kbd_
}

/*
** Prepare bits for next serial read(P1 and P2 data bits if applicable)
*/
void cu_ctr_clock_event(void)
{
// if(!cu_mouse_get_enabled())
  cu_ctr_blatch[0] >>= 1;
 if(!cu_kbd_get_enabled())
  cu_ctr_blatch[1] >>= 1;
}

/*
** Accepting previous and currently written PORTA value, processes hardware
** related to the Uzebox's controllers. Returns the new PINA value.
*/
auint cu_ctr_process(auint prev, auint curr)
{
 auint chg  = prev ^ curr;
 auint rise = chg  & curr;
 auint ret = 0;

// if(!cu_mouse_get_enabled())
  ret = ((cu_ctr_blatch[0] & 1U)     ); /* write P1 current data bit and overlay lightgun data if enabled */

 if(!cu_kbd_get_enabled())
  ret |= ((cu_ctr_blatch[1] & 1U) << 1); /* write P2 current data bit */

 if(rise == 0x04U)
  cu_ctr_latch_event();
 if(rise == 0x08U)
  cu_ctr_clock_event();

 return ret;
}



/*
** Sends current SNES controller state. The buttons are a combination of masks
** generated from the definitions.
*/
void  cu_ctr_setsnes(auint player, auint buttons)
{
 if (player > 1U){ return; }
 cu_ctr_buttons[player] = ~buttons;
}


#ifdef ENABLE_ICAP
#ifndef ENABLE_IREP
/*
** Retrieves current SNES controller state. The buttons are a combination of
** masks generated from the definitions.
*/
void  cu_ctr_getsnes(auint player, auint* buttons)
{
 if (player > 1U){ return; }
 *buttons = ~cu_ctr_buttons[player];
}
#endif
#endif
/*
** Sends state of single SNES controller button. The button is a single
** (non-mask) definition.
*/
void  cu_ctr_setsnes_single(auint player, auint button, boole press)
{
 auint binv;
 if (player > 1U){ return; }
 binv = cu_ctr_buttons[player];
 if (press){
  binv &= ~((auint)(1U) << button);
 }else{
  binv |=  ((auint)(1U) << button);
 }
 cu_ctr_buttons[player] = binv;
}
