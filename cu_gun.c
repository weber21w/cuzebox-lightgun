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


#include "cu_gun.h"
#include "guicore.h" /* for getting screen pixel data */

auint cu_gun_enabled[MAX_GUNS];
auint cu_gun_light[MAX_GUNS];
sint32 cu_gun_x[MAX_GUNS];
sint32 cu_gun_y[MAX_GUNS];
auint cu_gun_trigger[MAX_GUNS];
auint cu_gun_latched_data[MAX_GUNS];
SDL_Rect cu_gun_eye[MAX_GUNS]; /* a 1x1 rectangle used for reading from the Renderer(modeling the photo-diode) */
SDL_Surface *cu_gun_target_surface[MAX_GUNS]; /* a 1x1 pixel surface to read the Renderer pixel under the mouse into */
auint cu_gun_target_surface_initialized = 0;
SDL_Surface *cu_gun_cursor_image;
SDL_Cursor *cu_gun_cursor;

#define GUN_CURSOR_W 14
#define GUN_CURSOR_H 14
const uint32 cu_gun_cursor_pixel_data[GUN_CURSOR_W*GUN_CURSOR_H] = {
0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFFF0000,0xFFFF0000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFFF0000,0xFFFF0000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFFF0000,0xFFFF0000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFFF0000,0xFFFF0000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFFF0000,0xFFFF0000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFFF0000,0xFFFF0000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
0xFFFF0000,0xFFFF0000,0xFFFF0000,0xFFFF0000,0xFFFF0000,0xFFFF0000,0x00000000,0x00000000,0xFFFF0000,0xFFFF0000,0xFFFF0000,0xFFFF0000,0xFFFF0000,0xFFFF0000,
0xFFFF0000,0xFFFF0000,0xFFFF0000,0xFFFF0000,0xFFFF0000,0xFFFF0000,0x00000000,0x00000000,0xFFFF0000,0xFFFF0000,0xFFFF0000,0xFFFF0000,0xFFFF0000,0xFFFF0000,
0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFFF0000,0xFFFF0000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFFF0000,0xFFFF0000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFFF0000,0xFFFF0000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFFF0000,0xFFFF0000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFFF0000,0xFFFF0000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFFF0000,0xFFFF0000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
};

auint cu_gun_get_enabled(auint g){
 return cu_gun_enabled[g];
}


void cu_gun_set_enabled(auint val, auint g){
 if(cu_gun_enabled[g] != val){
  if(val){
   if(!cu_gun_target_surface_initialized)
    cu_gun_one_time_init(); /* create surfaces and load cursor/crosshair image */
   if(cu_gun_cursor != NULL)
    SDL_SetCursor(cu_gun_cursor);

    SDL_ShowCursor(SDL_ENABLE);
  }else
   SDL_SetCursor(SDL_GetDefaultCursor());

  cu_gun_enabled[g] = val;
 }
}


auint cu_gun_get_trigger(auint g){
 return cu_gun_trigger[g];
}


void cu_gun_set_trigger(auint val, auint g){
 cu_gun_trigger[g] = val;
}

void cu_gun_set_light(auint val, auint g){
 cu_gun_light[g] = val;
}

auint cu_gun_get_light(auint g){
 return cu_gun_light[g];
}

void cu_gun_get_position(sint32 *mx, sint32 *my, auint g){
 *mx = cu_gun_x[g];
 *my = cu_gun_y[g];
}


void cu_gun_set_position(sint32 mx, sint32 my, auint g){
 cu_gun_x[g] = mx;
 cu_gun_y[g] = my;
}

auint cu_gun_get_x(auint g){
 return cu_gun_x[g];
}

auint cu_gun_get_y(auint g){
 return cu_gun_y[g];
}

//extern SDL_Window*   guicore_window;
extern SDL_Renderer* guicore_renderer;

void cu_gun_one_time_init(){
  cu_gun_target_surface_initialized = 1U;
  cu_gun_cursor_image = SDL_CreateRGBSurfaceFrom(cu_gun_cursor_pixel_data,GUN_CURSOR_W,GUN_CURSOR_H,32,GUN_CURSOR_W*4,0x00FF0000U,0x0000FF00U,0x000000FFU,0xFF000000U);
//(0,7,7,32,0x00FF0000U,0x0000FF00U,0x000000FFU,0xFF000000U);
  cu_gun_cursor = SDL_CreateColorCursor(cu_gun_cursor_image,(GUN_CURSOR_W/2)-1,(GUN_CURSOR_H/2)-1); /* NULL if failed */
  auint g;
  for(g=0;g<MAX_GUNS;g++)
   cu_gun_target_surface[g] = SDL_CreateRGBSurfaceWithFormat(0,1,1,32,SDL_PIXELFORMAT_RGB888);
}

void cu_gun_latch_event(void){
 auint g;
 if(cu_gun_target_surface_initialized == 0U)
  cu_gun_one_time_init();

//cu_gun_enabled[0] = 1;
 for(g=0; g<MAX_GUNS; g++){
  if(!cu_gun_enabled[g])
   continue;
  
  cu_gun_trigger[g] = SDL_GetMouseState(cu_gun_x+g,cu_gun_y+g); /* update the postion of the gun to the host system mouse position(relative to the focused window) */
  float logical_x,logical_y;
  SDL_RenderWindowToLogical(guicore_renderer,cu_gun_x[g],cu_gun_y[g],&logical_x,&logical_y); /* convert window coordinates into logical coorindates for the resized renderer */
  cu_gun_eye[g].x = (sint32)(logical_x+0.5f);
  cu_gun_eye[g].y = (sint32)(logical_y+0.5f);
  cu_gun_eye[g].w = cu_gun_eye[g].h = 1U;

  SDL_RenderReadPixels(guicore_renderer,cu_gun_eye+g,SDL_PIXELFORMAT_RGB888,cu_gun_target_surface[g]->pixels,cu_gun_target_surface[g]->pitch); /* very slow... */
  cu_gun_light[g] = *(auint *)cu_gun_target_surface[g]->pixels;
  auint gray_light = (((cu_gun_light[g] & 0xFF0000)>>16U) + ((cu_gun_light[g] & 0x00FF00)>>8U) + ((cu_gun_light[g] & 0x0000FF)>>0U))/3;
  cu_gun_light[g] = (gray_light > 216)?1:0; /* this value catches the edges of pure white even with merging on */
  cu_gun_latched_data[g] = (cu_gun_light[g]<<13U);
  cu_gun_latched_data[g] |= (cu_gun_trigger[g]<<12U);
 }
}
