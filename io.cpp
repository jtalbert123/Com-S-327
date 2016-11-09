#include "characters.h"
#include "gameflow.h"
#include "pathfinding.h"
#include "io.h"
#include "items.hpp"

#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ncurses.h>

void init_color_pairs();

void monster_list_interface(dungeon_t* dungeon)
{
  int num_monsters = dungeon->num_characters-1;
  character** character_list = (character**)calloc(num_monsters, sizeof(character*));
  int r, c, i = 0;
  for (r = 0; r < DUNGEON_ROWS; r++)
  {
    for (c = 0; c < DUNGEON_COLS; c++)
    {
      character* character = dungeon->characters[r][c];
      if (character != NULL && character->type == MONSTER)
      {
	character_list[i++] = character;
	get_character_loc(character);
      }
    }
  }
  point_t loc = get_character_loc((character*)dungeon->pc);
  char **lines = (char**)malloc(sizeof(char*)*num_monsters);
  for (i = 0; i < num_monsters; i++)
  {
    lines[i] = (char*)malloc(sizeof(char)*35);
    character* monster = character_list[i];
    point_t mloc = get_character_loc(monster);
    point_t diff = { mloc.x - loc.x, mloc.y - loc.y };
    sprintf(lines[i], "| A %c, %2d %5s and %2d %4s |",
	    get_character_symbol(monster),
	    abs(diff.y),
	    (diff.y >= 0) ? "south" : "north",
	    abs(diff.x),
	    (diff.x >= 0) ? "east" : "west");
  }
  free(character_list);
  int offset = 0;
  const int max_display = 10;
  while (1)
  {
    //mvprintw((21-max_display)/2 - 1, (80-MAX_STRLEN)/2, "                             ");
    //mvprintw((21-max_display)/2 + max_display, (80-MAX_STRLEN)/2, "                             ");

    if (offset == 0)
    {
      mvprintw((21-max_display)/2 - 1, (80-29)/2, "-----------------------------");
    }
    else
    {
      mvprintw((21-max_display)/2 - 1, (80-29)/2, "--------------^--------------");
    }

    if (offset + max_display < num_monsters)
    {
      mvprintw((21-max_display)/2 + max_display, (80-29)/2, "--------------,--------------");
    }
    else
    {
      mvprintw((21-max_display)/2 + max_display, (80-29)/2, "-----------------------------");
    }

    int upper = max_display;
    if (num_monsters - offset < max_display)
    {
      upper = num_monsters - offset;
    }
    for (i = 0; i < upper; i++)
    {
      char* str = lines[i+offset];
      mvprintw(i+(21-max_display)/2, (80-strlen(str))/2, str);
    }
    for (i = upper; i < max_display; i++)
    {
      mvprintw(i+(21-max_display)/2, (80-29)/2, "|                           |");
    }
    refresh();
    int key = getch();
    if (key == 0402)
    {
      offset += 1;
      if (offset + max_display > num_monsters)
      {
	offset = num_monsters - max_display;
	if (offset < 0)
	{
	  offset = 0;
	}
      }
    }
    else if (key == 0403)
    {
      offset -= 1;
      if (offset < 0)
      {
	offset = 0;
      }
    }
    else if (key == 033)
    {
      break;
    }
  }
  for (i = 0; i < num_monsters; i++)
  {
    free(lines[i]);
  }
  free(lines);
  
  print_dungeon(dungeon);
}

void pc_turn_interface(dungeon_t* dungeon, player* pc)
{
  if (!pc->alive)
  {
    fprintf(stderr, "PC DEAD, BUT INTERFACE CALLED\n");
  }
  print_dungeon(dungeon);
  
  point_t loc = get_character_loc((character*)pc);

  /*
  int r, c, num = 0;
  for (r = 0; r < DUNGEON_ROWS; r++)
  {
    for (c = 0; c < DUNGEON_COLS; c++)
    {
      if (dungeon->characters[r][c] != NULL)
      {
	num++;
      }
    }
  }
  mvprintw(22, 0, "num_characters: %d", dungeon->num_characters);
  mvprintw(23, 0, "     (counted): %d", num);
  */
  int ch;
GETCHAR_LBL:
  ch = getch();
  switch (ch)
  {
  default:
    mvprintw(0, 0, "Unknown Key: %#o  ", ch);
    goto GETCHAR_LBL;
    break;

    /* numpad.7 (up+left) */
  case 067:
  case 'y':
    /* try move up+left */
    pc_try_move(dungeon, pc, -1, -1);
    break;

    /* numpad.8 (up) */
  case 070:
  case 'k':
    /* try move up */
    pc_try_move(dungeon, pc, 0, -1);
    break;

    /* numpad.9 (up+right) */
  case 071:
  case 'u':
    /* try move up+right */
    pc_try_move(dungeon, pc, 1, -1);
    break;

    /* numpad.1 (down+left) */
  case 061:
  case 'b':
    /* try move down+left */
    pc_try_move(dungeon, pc, -1, 1);
    break;

    /* numpad.2 (down) */
  case 062:
  case 'j':
    /* try move down */
    pc_try_move(dungeon, pc, 0, 1);
    break;

    /* numpad.3 (down+right) */
  case 063:
  case 'n':
    /* try move down+left */
    pc_try_move(dungeon, pc, 1, 1);
    break;

    /* numpad.4 (left) */
  case 064:
  case 'h':
    /* try move left */
    pc_try_move(dungeon, pc, -1, 0);
    break;

    /* numpad.g (right) */
  case 066:
  case 'l':
    /* try move left */
    pc_try_move(dungeon, pc, 1, 0);
    break;

  case 'Q':
    game_state.quitted = 1;
    game_state.running = 0;
    break;

  case 065:
  case ' ':
    /* numpad.5 */
    break;

  case 'm':
    monster_list_interface(dungeon);
    goto GETCHAR_LBL;
    break;

  case '<':
    if (dungeon->terrain[loc.y][loc.x] == UP_STAIR)
    {
      game_state.reload = 1;
    }
    else
    {
      goto GETCHAR_LBL;
    }
    break;

  case '>':
    if (dungeon->terrain[loc.y][loc.x] == DOWN_STAIR)
    {
      game_state.reload = 1;
    }
    else
    {
      goto GETCHAR_LBL;
    }
    break;
  }
}

void print_dungeon(dungeon_t* dungeon)
{
  PlayerMemory memory = get_pc_memory(dungeon->pc);
  point_t loc = get_character_loc((character*)dungeon->pc);
  int r, c;
  for (r = 0; r < DUNGEON_ROWS; r++)
  {
    for (c = 0; c < DUNGEON_COLS; c++)
    {
      tile_type tile = memory.terrain[r][c];
      item* item = memory.items[r][c];
      //tile_type tile = dungeon->terrain[r][c];
      char ch = ' ';
      int color = COLOR_WHITE;
      if (dungeon->characters[r][c] != NULL && (abs(loc.x - c) <= 3) && (abs(loc.y - r) <= 3))
      {
	character* character = dungeon->characters[r][c];
        ch = get_character_symbol(character);
	color = character->colors[0];
      }
      else if (item != NULL)
      {
	ch = item->get_symbol();
	color = item->get_colors()[0];
      }
      else if (tile == WALL)
      {
	ch = ' ';
      }
      else if (tile == FLOOR)
      {
        ch = '.';
      }
      else if (tile == HALL)
      {
        ch = '#';
      }
      else if (tile == UP_STAIR)
      {
        ch = '<';
      }
      else if (tile == DOWN_STAIR)
      {
        ch = '>';
      }
      attron(COLOR_PAIR(color));
      mvaddch(r, c, ch);
      attroff(COLOR_PAIR(color));
    }
  }
  refresh();
}

void init_io()
{
  initscr();
  raw();
  keypad(stdscr, TRUE);
  noecho();
  set_escdelay(100);
  start_color();
  init_color_pairs();
}

void init_color_pairs()
{
  init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
  init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
  init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
  init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
  init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
  init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
  init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
}

void end_io()
{
  endwin();
}
