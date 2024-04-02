/*
 *  SNES Mouse Emulation
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


#include "cu_mouse.h"
#include "cu_gun.h"

auint cu_mouse_enabled;
sint32 cu_mouse_dx, cu_mouse_dy;
auint cu_mouse_buttons; /* buffer to accumulate first 16 bits of serial data after a Latch event(buttons+signature) */
auint cu_mouse_delta; /* buffer to accumulate the last 16 bits of serial data after a Latch event(x+y axis movement deltas) */
auint cu_mouse_latched_data; /* set upon Latch event: data to be read out serially with each Clock event */
auint cu_mouse_scale; /* sensitivity of the SNES mouse(not supported on Hyperkin) */
auint cu_mouse_trap_host_cursor;

void cu_mouse_adjust_scale(){ /* TODO have keyboard ctrl+alt+m toggle mouse sensitivity */
 if(cu_mouse_scale == 0U && !cu_gun_get_enabled(0)){ /* mouse was previously disabled, but hasn't cycled through gun enable/disable? */
  cu_gun_set_enabled(1U,0U);
  print_unf("Lightgun emulation enabled(iterate mouse sensitivity to disable)\n");
  return;
 }else if(cu_mouse_scale == 0U){ /* gun was already cycled on? */
  cu_gun_set_enabled(0U,0U);
  cu_mouse_scale++;
  print_unf("Lightgun emulation disabled(iterate mouse sensitivity to enable)\n");
 }else{	
   if(!cu_mouse_enabled){ /* mouse was previously disabled */
    cu_mouse_enabled = 1U;
    print_unf("SNES Mouse Enabled(overriding P1)\n");
    SDL_ShowCursor(SDL_DISABLE);
    cu_gun_set_enabled(0,0);
   }else if(++cu_mouse_scale == 6U){ /* wrapped around sensitivity, disable(user can re-enable and go back through scale) */
    cu_mouse_enabled = 0U;
    cu_mouse_scale = 0U;
    print_unf("SNES Mouse Disabled(P1 restored)\n");
    SDL_ShowCursor(SDL_ENABLE);
   }else
    print_message("SNES Mouse Sensitivity: %d\n", cu_mouse_scale);
  }
}


auint cu_mouse_encode_delta(sint32 d){
 uint8 result;
 if (d < 0){
   result = 0;
   d = -d;
 }else
  result = 1;

 if (d > 127)
  d = 127;
 if (!(d & 64))
  result |= 2;
 if (!(d & 32))
  result |= 4;
 if (!(d & 16))
  result |= 8;
 if (!(d & 8))
  result |= 16;
 if (!(d & 4))
  result |= 32;
 if (!(d & 2))
  result |= 64;
 if (!(d & 1))
  result |= 128;
 return result & 0xFF;
}


void cu_mouse_latch_event(){
 if(!cu_mouse_enabled) /* this shouldn't happen... */
  return;
}


auint cu_mouse_process(auint prev, auint curr){
/*
 buttons[0] = (encode_delta(cu_mouse_dx) << 24) | (encode_delta(cu_mouse_dy) << 16) | 0x7FFF;
 if (cu_mouse_buttons & SDL_BUTTON_LMASK)
  buttons[0] &= ~(1<<9);
 if (cu_mouse_buttons & SDL_BUTTON_RMASK)
  buttons[0] &= ~(1<<8);
*/
 return curr;
}


auint cu_mouse_get_enabled(void){
 return cu_mouse_enabled;
}


void cu_mouse_set_enabled(auint val){
 cu_mouse_enabled = val;
 SDL_ShowCursor(val?SDL_DISABLE:SDL_ENABLE);
}


auint cu_mouse_get_buttons(void){
 return cu_mouse_buttons & CU_MOUSE_SIGNATURE;
}


void cu_mouse_set_buttons(auint val){
 cu_mouse_buttons = val;
}

void cu_mouse_accumulate_buttons(auint val){
 cu_mouse_buttons |= val;
}

void cu_mouse_accumulate_deltas(sint32 mx, sint32 my){

}

auint cu_mouse_get_scale(void){
 return cu_mouse_scale;
}


void cu_mouse_set_scale(auint val){
 cu_mouse_scale = val;
}


void cu_mouse_get_position(sint32 *mx, sint32 *my){
 *mx = cu_mouse_dx;
 *my = cu_mouse_dy;
}


void cu_mouse_set_position(sint32 mx, sint32 my){
 cu_mouse_dx = mx;
 cu_mouse_dy = my;
}

auint cu_mouse_get_trap_host_cursor(void){
 return cu_mouse_trap_host_cursor;
}

void cu_mouse_set_trap_host_cursor(auint val){
 cu_mouse_trap_host_cursor = val;
}
