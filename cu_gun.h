/*
 *  Lightgun Emulation
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

#ifndef GUN_H
#define GUN_H

#include "types.h"

/* The Uzebox lightgun is based on a modified NES Zapper(or off brand like Tomee). The Tomee gun in particular has no PCB at all, and simply
 * has the light sensor connected to one wire, and the trigger to another. The Uzebox does not have the necessary wires in the controller port
 * to do things the way the NES did. Instead these wires are converted into serial data, on the unused bits of the standard SNES joypad bitmap.
 * Software can detect the gun by a signature of 0b11 instead of the joypad's 0b00. This has the implication that you can have a lightgun with
 * up to 12 buttons corresponding to SNES joypad buttons. Because of that, the handling is special and intertwinned with the joypad emulation.
 * If the gun(s) are enabled, the extra data will be logically OR'ed with the existing joypad(s) data.
*/

/*
** Color levels that determine if light is sense or not
*/

#define MAX_GUNS 2
#define CU_GUN_SENSE_R 128
#define CU_GUN_SENSE_G 128
#define CU_GUN_SENSE_B 128

#define CU_GUN_TRIGGER (1U<<12)
#define CU_GUN_LIGHT (1U<<13)
extern auint cu_gun_enabled[MAX_GUNS]; /* if enabled, controller emulation will request sense and trigger state */
extern auint cu_gun_light[MAX_GUNS]; /* if the pixel under the cursor(emulated gun position) is light or not */
extern auint cu_gun_trigger[MAX_GUNS]; /* 0 = not pressed, 1 = pressed */
extern sint32 cu_gun_x[MAX_GUNS]; /* position relative to the window for the gun cursor(for pixel sampling and cursor display) */
extern sint32 cu_gun_y[MAX_GUNS];
extern auint cu_gun_latched_data[MAX_GUNS];

/*
** Determine if the pixel under the gun pointer is: 0="dark" or 1="light"
*/
auint  cu_gun_sense(auint g);

/*
** Convert system mouse events into emulated lightgun position(for determining pixel "lightness")
*/
void  cu_gun_update();

/*
** Handle latch/clock events for the lightgun data
*/
auint cu_gun_process(auint prev, auint curr);


/*
** Get the lightgun enabled status
*/
auint  cu_gun_get_enabled(auint g);


/*
** Set the lightgun status to enabled
*/
void  cu_gun_set_enabled(auint val, auint g);


/*
** Get the lightgun trigger status
*/
auint  cu_gun_get_trigger(auint g);


/*
** Set the lightgun trigger to enabled
*/
void  cu_gun_set_trigger(auint val, auint g);


/*
** Get the lightgun sense status(0 or 1)
*/
auint  cu_gun_get_light(auint g);


/*
** Set the lightgun sense status(0 or 1)
*/
void  cu_gun_set_light(auint val, auint g);

/*
** Get the lightgun position
*/
void  cu_gun_get_position(sint32 *mx, sint32 *my, auint g);


/*
** Set the lightgun position
*/
void  cu_gun_set_position(sint32 mx, sint32 my, auint g);

void cu_gun_latch_event(void);

auint cu_gun_get_x(auint g);
auint cu_gun_get_y(auint g);
void cu_gun_one_time_init();

#endif /* GUN_H */
