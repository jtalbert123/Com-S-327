#ifndef _IO_H_
# define _IO_H_

#include "characters.h"
#include "dungeons.h"

void pc_turn_interface(dungeon_t* dungeon, pc_t* pc);
void print_dungeon(dungeon_t* dungeon);
void init_io();
void end_io();

#endif //_IO_H_