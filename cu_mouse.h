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

#ifndef MOUSE_H
#define MOUSE_H

#include "types.h"

#define CU_MOUSE_SIGNATURE (1U<<15U)
#define CU_MOUSE_LEFT_BUTTON (1U<<1U)
#define CU_MOUSE_RIGHT_BUTTON (1U<<2U)

extern auint cu_mouse_enabled;
extern sint32 cu_mouse_dx, cu_mouse_dy;
extern auint cu_mouse_buttons;
extern auint cu_mouse_scale;
extern auint cu_mouse_latched_data;
extern auint cu_mouse_trap_host_cursor;


/*
** Return whether or not the host system cursor should be trapped in the window
*/
auint  cu_mouse_get_trap_host_cursor(void);


/*
** Set whether or not the host system cursor should be trapped in the window(cursor is hidden if so)
*/
void cu_mouse_set_trap_host_cursor(auint val);


/*
** Cycle through sensitivity(also disable/enable by going forward)
*/
void  cu_mouse_adjust_scale();


/*
** Handle latch/clock events for the SNES mouse data
*/
auint  cu_mouse_process(auint prev, auint curr);


/*
** Get the mouse enabled status
*/
auint  cu_mouse_get_enabled(void);


/*
** Set the mouse status to enabled
*/
void  cu_mouse_set_enabled(auint val);


/*
** Handle Latch rise event
*/
void cu_mouse_latch_event(void);


/*
** Accumulate host system mouse presses
*/
void  cu_mouse_accumulate_buttons(auint val);


/*
** Reset the mouse deltas for both axis
*/
void  cu_mouse_reset_deltas(void);


/*
** Accumulate the mouse deltas
*/
void  cu_mouse_accumulate_deltas(sint32 mx, sint32 my);


/*
** Get the mouse position
*/
void  cu_mouse_get_position(sint32 *mx, sint32 *my);


/*
** Set the mouse position
*/
void  cu_mouse_set_position(sint32 mx, sint32 my);


/*
** Get the mouse buttons
*/
auint  cu_mouse_get_buttons(void);



/*
** Set the mouse buttons
*/
void  cu_mouse_set_buttons(auint val);



/*
** Get the mouse scale
*/
auint  cu_mouse_get_scale(void);


/*
** Set the mouse scale
*/
void  cu_mouse_set_scale(auint val);

#endif /* MOUSE_H */
