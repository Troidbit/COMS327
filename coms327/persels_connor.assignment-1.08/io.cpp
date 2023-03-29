#include <unistd.h>
#include <ncurses.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <vector>

#include "io.h"
#include "character.h"
#include "poke327.h"
#include "pokemon.h"
#include "db_parse.h"

typedef struct io_message {
  /* Will print " --more-- " at end of line when another message follows. *
   * Leave 10 extra spaces for that.                                      */
  char msg[71];
  struct io_message *next;
} io_message_t;

static io_message_t *io_head, *io_tail;

void io_init_terminal(void)
{
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  start_color();
  init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
  init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
  init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
  init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
  init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
  init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
}

void io_reset_terminal(void)
{
  endwin();

  while (io_head) {
    io_tail = io_head;
    io_head = io_head->next;
    free(io_tail);
  }
  io_tail = NULL;
}

void io_queue_message(const char *format, ...)
{
  io_message_t *tmp;
  va_list ap;

  if (!(tmp = (io_message_t *) malloc(sizeof (*tmp)))) {
    perror("malloc");
    exit(1);
  }

  tmp->next = NULL;

  va_start(ap, format);

  vsnprintf(tmp->msg, sizeof (tmp->msg), format, ap);

  va_end(ap);

  if (!io_head) {
    io_head = io_tail = tmp;
  } else {
    io_tail->next = tmp;
    io_tail = tmp;
  }
}

static void io_print_message_queue(uint32_t y, uint32_t x)
{
  while (io_head) {
    io_tail = io_head;
    attron(COLOR_PAIR(COLOR_CYAN));
    mvprintw(y, x, "%-80s", io_head->msg);
    attroff(COLOR_PAIR(COLOR_CYAN));
    io_head = io_head->next;
    if (io_head) {
      attron(COLOR_PAIR(COLOR_CYAN));
      mvprintw(y, x + 70, "%10s", " --more-- ");
      attroff(COLOR_PAIR(COLOR_CYAN));
      refresh();
      getch();
    }
    free(io_tail);
  }
  io_tail = NULL;
}

/**************************************************************************
 * Compares trainer distances from the PC according to the rival distance *
 * map.  This gives the approximate distance that the PC must travel to   *
 * get to the trainer (doesn't account for crossing buildings).  This is  *
 * not the distance from the NPC to the PC unless the NPC is a rival.     *
 *                                                                        *
 * Not a bug.                                                             *
 **************************************************************************/
static int compare_trainer_distance(const void *v1, const void *v2)
{
  const Character *const *c1 = (const Character *const *) v1;
  const Character *const *c2 = (const Character *const *) v2;

  return (world.rival_dist[(*c1)->pos[dim_y]][(*c1)->pos[dim_x]] -
          world.rival_dist[(*c2)->pos[dim_y]][(*c2)->pos[dim_x]]);
}

static Character *io_nearest_visible_trainer()
{
  Character **c, *n;
  uint32_t x, y, count;

  c = (Character **) malloc(world.cur_map->num_trainers * sizeof (*c));

  /* Get a linear list of trainers */
  for (count = 0, y = 1; y < MAP_Y - 1; y++) {
    for (x = 1; x < MAP_X - 1; x++) {
      if (world.cur_map->cmap[y][x] && world.cur_map->cmap[y][x] !=
          &world.pc) {
        c[count++] = world.cur_map->cmap[y][x];
      }
    }
  }

  /* Sort it by distance from PC */
  qsort(c, count, sizeof (*c), compare_trainer_distance);

  n = c[0];

  free(c);

  return n;
}

void io_display()
{
  uint32_t y, x;
  Character *c;

  clear();
  for (y = 0; y < MAP_Y; y++) {
    for (x = 0; x < MAP_X; x++) {
      if (world.cur_map->cmap[y][x]) {
        mvaddch(y + 1, x, world.cur_map->cmap[y][x]->symbol);
      } else {
        switch (world.cur_map->map[y][x]) {
        case ter_boulder:
        case ter_mountain:
          attron(COLOR_PAIR(COLOR_MAGENTA));
          mvaddch(y + 1, x, '%');
          attroff(COLOR_PAIR(COLOR_MAGENTA));
          break;
        case ter_tree:
        case ter_forest:
          attron(COLOR_PAIR(COLOR_GREEN));
          mvaddch(y + 1, x, '^');
          attroff(COLOR_PAIR(COLOR_GREEN));
          break;
        case ter_path:
        case ter_exit:
          attron(COLOR_PAIR(COLOR_YELLOW));
          mvaddch(y + 1, x, '#');
          attroff(COLOR_PAIR(COLOR_YELLOW));
          break;
        case ter_mart:
          attron(COLOR_PAIR(COLOR_BLUE));
          mvaddch(y + 1, x, 'M');
          attroff(COLOR_PAIR(COLOR_BLUE));
          break;
        case ter_center:
          attron(COLOR_PAIR(COLOR_RED));
          mvaddch(y + 1, x, 'C');
          attroff(COLOR_PAIR(COLOR_RED));
          break;
        case ter_grass:
          attron(COLOR_PAIR(COLOR_GREEN));
          mvaddch(y + 1, x, ':');
          attroff(COLOR_PAIR(COLOR_GREEN));
          break;
        case ter_clearing:
          attron(COLOR_PAIR(COLOR_GREEN));
          mvaddch(y + 1, x, '.');
          attroff(COLOR_PAIR(COLOR_GREEN));
          break;
        default:
 /* Use zero as an error symbol, since it stands out somewhat, and it's *
  * not otherwise used.                                                 */
          attron(COLOR_PAIR(COLOR_CYAN));
          mvaddch(y + 1, x, '0');
          attroff(COLOR_PAIR(COLOR_CYAN)); 
       }
      }
    }
  }

  mvprintw(23, 1, "PC position is (%2d,%2d) on map %d%cx%d%c.",
           world.pc.pos[dim_x],
           world.pc.pos[dim_y],
           abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)),
           world.cur_idx[dim_x] - (WORLD_SIZE / 2) >= 0 ? 'E' : 'W',
           abs(world.cur_idx[dim_y] - (WORLD_SIZE / 2)),
           world.cur_idx[dim_y] - (WORLD_SIZE / 2) <= 0 ? 'N' : 'S');
  mvprintw(22, 1, "%d known %s.", world.cur_map->num_trainers,
           world.cur_map->num_trainers > 1 ? "trainers" : "trainer");
  mvprintw(22, 30, "Nearest visible trainer: ");
  if ((c = io_nearest_visible_trainer())) {
    attron(COLOR_PAIR(COLOR_RED));
    mvprintw(22, 55, "%c at %d %c by %d %c.",
             c->symbol,
             abs(c->pos[dim_y] - world.pc.pos[dim_y]),
             ((c->pos[dim_y] - world.pc.pos[dim_y]) <= 0 ?
              'N' : 'S'),
             abs(c->pos[dim_x] - world.pc.pos[dim_x]),
             ((c->pos[dim_x] - world.pc.pos[dim_x]) <= 0 ?
              'W' : 'E'));
    attroff(COLOR_PAIR(COLOR_RED));
  } else {
    attron(COLOR_PAIR(COLOR_BLUE));
    mvprintw(22, 55, "NONE.");
    attroff(COLOR_PAIR(COLOR_BLUE));
  }

  io_print_message_queue(0, 0);

  refresh();
}

uint32_t io_teleport_pc(pair_t dest)
{
  /* Just for fun. And debugging.  Mostly debugging. */

  do {
    dest[dim_x] = rand_range(1, MAP_X - 2);
    dest[dim_y] = rand_range(1, MAP_Y - 2);
  } while (world.cur_map->cmap[dest[dim_y]][dest[dim_x]]                  ||
           move_cost[char_pc][world.cur_map->map[dest[dim_y]]
                                                [dest[dim_x]]] == INT_MAX ||
           world.rival_dist[dest[dim_y]][dest[dim_x]] < 0);

  return 0;
}

static void io_scroll_trainer_list(char (*s)[40], uint32_t count)
{
  uint32_t offset;
  uint32_t i;

  offset = 0;

  while (1) {
    for (i = 0; i < 13; i++) {
      mvprintw(i + 6, 19, " %-40s ", s[i + offset]);
    }
    switch (getch()) {
    case KEY_UP:
      if (offset) {
        offset--;
      }
      break;
    case KEY_DOWN:
      if (offset < (count - 13)) {
        offset++;
      }
      break;
    case 27:
      return;
    }

  }
}

static void io_list_trainers_display(Npc **c,
                                     uint32_t count)
{
  uint32_t i;
  char (*s)[40]; /* pointer to array of 40 char */

  s = (char (*)[40]) malloc(count * sizeof (*s));

  mvprintw(3, 19, " %-40s ", "");
  /* Borrow the first element of our array for this string: */
  snprintf(s[0], 40, "You know of %d trainers:", count);
  mvprintw(4, 19, " %-40s ", s[0]);
  mvprintw(5, 19, " %-40s ", "");

  for (i = 0; i < count; i++) {
    snprintf(s[i], 40, "%16s %c: %2d %s by %2d %s",
             char_type_name[c[i]->ctype],
             c[i]->symbol,
             abs(c[i]->pos[dim_y] - world.pc.pos[dim_y]),
             ((c[i]->pos[dim_y] - world.pc.pos[dim_y]) <= 0 ?
              "North" : "South"),
             abs(c[i]->pos[dim_x] - world.pc.pos[dim_x]),
             ((c[i]->pos[dim_x] - world.pc.pos[dim_x]) <= 0 ?
              "West" : "East"));
    if (count <= 13) {
      /* Handle the non-scrolling case right here. *
       * Scrolling in another function.            */
      mvprintw(i + 6, 19, " %-40s ", s[i]);
    }
  }

  if (count <= 13) {
    mvprintw(count + 6, 19, " %-40s ", "");
    mvprintw(count + 7, 19, " %-40s ", "Hit escape to continue.");
    while (getch() != 27 /* escape */)
      ;
  } else {
    mvprintw(19, 19, " %-40s ", "");
    mvprintw(20, 19, " %-40s ",
             "Arrows to scroll, escape to continue.");
    io_scroll_trainer_list(s, count);
  }

  free(s);
}

static void io_list_trainers()
{
  Character **c;
  uint32_t x, y, count;

  c = (Character **) malloc(world.cur_map->num_trainers * sizeof (*c));

  /* Get a linear list of trainers */
  for (count = 0, y = 1; y < MAP_Y - 1; y++) {
    for (x = 1; x < MAP_X - 1; x++) {
      if (world.cur_map->cmap[y][x] && world.cur_map->cmap[y][x] !=
          &world.pc) {
        c[count++] = world.cur_map->cmap[y][x];
      }
    }
  }

  /* Sort it by distance from PC */
  qsort(c, count, sizeof (*c), compare_trainer_distance);

  /* Display it */
  io_list_trainers_display((Npc **)(c), count);
  free(c);

  /* And redraw the map */
  io_display();
}

void io_pokemart()
{
  mvprintw(0, 0, "Welcome to the Pokemart.  Could I interest you in some Pokeballs?");
  refresh();
  getch();
}

void io_pokemon_center()
{
  mvprintw(0, 0, "Welcome to the Pokemon Center.  How can Nurse Joy assist you?");
  refresh();
  getch();
}

void give_trainer_pokemon(Npc *npc) {
    int md = (abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)) +
        abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)));
    int minl, maxl;

    if (md <= 200) {
        minl = 1;
        maxl = md / 2;
    }
    else {
        minl = (md - 200) / 2;
        maxl = 100;
    }
    if (minl < 1) {
        minl = 1;
    }
    if (minl > 100) {
        minl = 100;
    }
    if (maxl < 1) {
        maxl = 1;
    }
    if (maxl > 100) {
        maxl = 100;
    }

    Pokemon p = Pokemon(rand() % (maxl - minl + 1) + minl);
    npc->pokemons.push_back(p);

    while ((rand() % 100) >= 60 && npc->pokemons.size() < 6) {
         Pokemon newP = Pokemon(rand() % (maxl - minl + 1) + minl);
         npc->pokemons.push_back(newP);
    }
}

int io_next_available_poke(Character *c) {
    int i=-1;
    for (i = 0; i < (int) c->pokemons.size(); i++) {
        if (c->pokemons.at(i).currenthp > 0) {
            return i;
        }
    }
    return i;
}

void io_display_pokemon_stat(WINDOW* win, Pokemon* p) {
    werase(win);
    int i;
    wprintw(win, "%s\nLVL: %d\nHP: %d\nATK: %d\nDEF: %d\n\n", p->get_species(), p->level, p->get_maxhp(), p->get_atk(), p->get_def());
    wprintw(win, "moves:\n");
    for (i = 0; i < 4; i++) {
        wprintw(win, "->%s\n", p->get_move(i));
    }
    wrefresh(win);
}

void io_display_bag(Pokemon *pc_poke) {
    int i;
    char c;
    int height = 24;
    int width = 80;
    WINDOW* win = newwin(height, width, 0, 0);
    box(win, '#', '#');
    mvwprintw(win, 1, (width / 2) - 5, "your bag!!");
    mvwhline(win, 2, 0, '#', 80);
    wrefresh(win);
    mvwprintw(win, 5, 2, "1. Potions: %d\n", world.pc.bag[potions]);
    mvwprintw(win, 7, 2, "2. Revives: %d\n", world.pc.bag[revives]);
    mvwprintw(win, 10, 2, "3. cancel");
    wrefresh(win);

    switch (c = getchar()) {
        case '1':
            {
                if (world.pc.bag[potions] > 0) {
                    clear();
                    printw("Choose who to heal: \n\n");
                    printw("1. %s", pc_poke->get_species());
                    for (i = 0; i < (int)world.pc.pokemons.size(); i++) {
                        printw("%d. %s", i + 2, world.pc.pokemons.at(i).get_species());
                    }

                    int healed = getch() - '0';
                    if (healed < (int)world.pc.pokemons.size() + 2) {
                        if (healed == 1) {
                            pc_poke->currenthp += 20;
                        }
                        else {
                            world.pc.pokemons.at(healed - 2).currenthp += 20;
                        }

                    }
                    world.pc.bag[potions] -= 1;
                }
            }
            break;

        case '2':
            {
                if (world.pc.bag[revives] > 0) {
                    clear();
                    printw("Choose who to revive: \n\n");
                    for (i = 0; i < (int)world.pc.pokemons.size(); i++) {
                        if (world.pc.pokemons.at(i).currenthp <= 0) {
                            printw("%d. %s", i+1, world.pc.pokemons.at(i).get_species());
                        }
                    }
                    printw("%d. cancel", i+1);

                    int revived = getch() - '0';
                    if (revived-1 == i) {
                        werase(win);
                        return;
                    }
                    else {
                        world.pc.pokemons.at(revived - 1).currenthp += world.pc.pokemons.at(revived-1).get_maxhp()/2;
                    }
                    world.pc.bag[revives] -= 1;
                }
            }
            break;
        case '3':
        {}
            werase(win);
            return;
            break;
    }
    werase(win);
}

void io_display_poke_switch(Pokemon* pc_poke) {
    int i, c;
    int height = 24;
    int width = 80;
    WINDOW* win = newwin(height, width, 0, 0);
    werase(win);
    box(win, '#', '#');
    mvwprintw(win, 1, (width / 2) - 11, "switch your pokemon!!");
    mvwhline(win, 2, 0, '#', 80);

    for (i = 0; i < (int)world.pc.pokemons.size(); i++) {
        mvwprintw(win, 4 + i, 2, "%d. %s\n", i + 1, world.pc.pokemons.at(i + 1).get_species());
    }
    mvwprintw(win,4+i,2, "%d. cancel\n", i+1);
    wrefresh(win);

    c = getch() - '0';
    if (c == i) {
        werase(win);
        return;
    }
    else {
        if (c - 1 < (int)world.pc.pokemons.size()) {
            Pokemon temp = *pc_poke;
            world.pc.pokemons.push_back(temp);
            Pokemon temp2 = world.pc.pokemons.at(c - 1);
            *pc_poke = temp2;
            world.pc.pokemons.erase(world.pc.pokemons.begin() + (c - 1));
        }
    }
    werase(win);
}

void io_battle_boarder_draw(WINDOW* win) {
    werase(win);
    int width = 80;
    box(win, '#', '#');
    mvwprintw(win, 1, (width / 2) - 11, "You are in battle!!!!!");
    mvwhline(win, 2, 0, '#', 80);
    wrefresh(win);
}

void io_battle_left_draw(WINDOW* win, Pokemon* p) {
    werase(win);
    wprintw(win, "1. Attack with %s\n\n2. Switch out pokemon\n\n3. Open bag\n",p->get_species());
    wrefresh(win);
}

int io_pc_attack(Pokemon* pc_poke,Pokemon* t_poke) {
    int i,c;
    int height = 24;
    int width = 80;
    WINDOW* win = newwin(height, width, 0, 0);
    werase(win);
    box(win, '#', '#');
    mvwprintw(win, 1, (width / 2) - 11, "You are in battle!!!!!");
    mvwhline(win, 2, 0, '#', 80);
    
    mvwprintw(win,4,2, "moves:\n");
    for (i = 0; i < 4; i++) {
        if (pc_poke->get_move(i)) {
            mvwprintw(win, 5 + i, 2, "%d. %s", i + 1, moves[pc_poke->move_index[i]].identifier);
        }
    }
    
    
    wrefresh(win);
    do {
        c = getch() - '0';
    } while (c > 4);
    
    move_db move = moves[pc_poke->move_index[c - 1]];
    int randy = rand() % (100 - 85 + 1) + 85;



    if (rand() % (100 - move.accuracy) == 0) {
        werase(win);
        return (((((((2 * pc_poke->level) / 5) + 2) * move.power) * (pc_poke->get_atk() / t_poke->get_def())) / 50) + 2)* randy;
    }
    else {
        werase(win);
        return 0;
    }
}

/*
* Returns dmg the pc will deal to pokemon
*/
int io_battle_input(Pokemon* pc_poke, Pokemon* t_poke) {
    char c;
    switch (c = getch()) {
    case '1':
        if (pc_poke->currenthp <= 0) { return io_battle_input(pc_poke,t_poke); }
        io_pc_attack(pc_poke,t_poke);
        break;
    case '2':
        io_display_poke_switch(pc_poke);
        break;
    case '3':
        io_display_bag(pc_poke);
        break;
    default:
        return io_battle_input(pc_poke,t_poke);
        break;
    }
    return 0;
}

void io_battle_loop(Npc *npc, Pc *pc, Pokemon *t_poke, Pokemon *pc_poke,WINDOW* win, WINDOW* win_left, WINDOW* win_mid, WINDOW* win_right) {
    int pc_dmg, t_dmg;
    do {
        io_battle_boarder_draw(win);
        io_battle_left_draw(win_left,pc_poke);
        io_display_pokemon_stat(win_mid, pc_poke);
        io_display_pokemon_stat(win_right,t_poke);

        pc_dmg = 0;
        t_dmg = 0;

        pc_dmg = io_battle_input(pc_poke,t_poke);
        printw("%d", pc_dmg);

        
        move_db t_move = moves[t_poke->move_index[0]];
        int randy = rand() % (100 - 85 + 1) + 85;
        t_dmg = (((((((2 * t_poke->level) / 5) + 2) * t_move.power) * (t_poke->get_atk() / pc_poke->get_def())) / 50) + 2)* randy;

        if (pc_poke->get_speed() < t_poke->get_speed()) {
            t_poke->currenthp -= pc_dmg;
            if (t_poke->currenthp <= 0) {

            }
            pc_poke->currenthp -= t_dmg;
            if (pc_poke->currenthp <= 0) {

            }
        }
        else {
            pc_poke->currenthp -= t_dmg;
            t_poke->currenthp -= pc_dmg;
        }
    } while (true);
}

void io_battle(Character *aggressor, Character *defender)
{
  Npc *npc;
  if (!(npc = dynamic_cast<Npc*>(aggressor))) {
      npc = dynamic_cast<Npc*>(defender);
  }

  int height = 24;
  int width = 80;
  int a;
  WINDOW* win = newwin(height, width, 0, 0);
  WINDOW* win_left  = newwin(height - 5, (width / 3)-1, 4, 2);
  WINDOW* win_mid   = newwin(height - 5, (width / 3)-1, 4, (width / 3) + 2);
  WINDOW* win_right = newwin(height - 5, (width / 3)-1, 4, ((width * 2) / 3) + 1);

  
  a = io_next_available_poke(&world.pc);
  if (a == -1) {
      return;
  }
  Pokemon pc_poke = world.pc.pokemons.at(a);
  world.pc.pokemons.erase(world.pc.pokemons.begin() + a);

  give_trainer_pokemon(npc);
  a = io_next_available_poke(npc);
  Pokemon trainer_poke = npc->pokemons.at(a);
  npc->pokemons.erase(npc->pokemons.begin() + a);

  //...
  io_battle_loop(npc,&world.pc,&trainer_poke,&pc_poke,win,win_left,win_mid,win_right);


  //trainer sad now cuz he lost
  npc->defeated = 1;
  if (npc->ctype == char_hiker || npc->ctype == char_rival) {
    npc->mtype = move_wander;
  }
}

void io_display_pokepick(void) {
    
    Pokemon p1 = Pokemon(5);
    Pokemon p2 = Pokemon(5);
    Pokemon p3 = Pokemon(5);

    WINDOW* win;
    int height = 14;
    int width = 73;

    win = newwin(height, width, 5, 4);
    box(win, height / 2, width / 2);
    mvprintw(0, 0, "Pokemon 327 game!");
    mvprintw(1, 0, "Pick your starter pokemon:");
    mvprintw(3, 0, "1: %s\n2: %s\n3: %s\n\nClick the corosponding number on your keyboard", p1.get_species(), p2.get_species(), p3.get_species());
    wrefresh(win);
    int c;
    do {
        switch (c = getch()) {
            case '1':
                world.pc.pokemons.push_back(p1);
                break;
            case '2':
                world.pc.pokemons.push_back(p2);
                break;
            case '3':
                world.pc.pokemons.push_back(p3);
                break;
            default:
                break;
        }
    } while (c != '1' && c != '2' && c != '3');
    endwin();
}

uint32_t move_pc_dir(uint32_t input, pair_t dest)
{
  dest[dim_y] = world.pc.pos[dim_y];
  dest[dim_x] = world.pc.pos[dim_x];

  switch (input) {
  case 1:
  case 2:
  case 3:
    dest[dim_y]++;
    break;
  case 4:
  case 5:
  case 6:
    break;
  case 7:
  case 8:
  case 9:
    dest[dim_y]--;
    break;
  }
  switch (input) {
  case 1:
  case 4:
  case 7:
    dest[dim_x]--;
    break;
  case 2:
  case 5:
  case 8:
    break;
  case 3:
  case 6:
  case 9:
    dest[dim_x]++;
    break;
  case '>':
    if (world.cur_map->map[world.pc.pos[dim_y]][world.pc.pos[dim_x]] ==
        ter_mart) {
      io_pokemart();
    }
    if (world.cur_map->map[world.pc.pos[dim_y]][world.pc.pos[dim_x]] ==
        ter_center) {
      io_pokemon_center();
    }
    break;
  }

  if ((world.cur_map->map[dest[dim_y]][dest[dim_x]] == ter_exit) &&
      (input == 1 || input == 3 || input == 7 || input == 9)) {
    // Exiting diagonally leads to complicated entry into the new map
    // in order to avoid INT_MAX move costs in the destination.
    // Most easily solved by disallowing such entries here.
    return 1;
  }

  if (world.cur_map->cmap[dest[dim_y]][dest[dim_x]]) {
    if (dynamic_cast<Npc *>(world.cur_map->cmap[dest[dim_y]][dest[dim_x]]) &&
        ((Npc *) world.cur_map->cmap[dest[dim_y]][dest[dim_x]])->defeated) {
      // Some kind of greeting here would be nice
      return 1;
    } else if (dynamic_cast<Npc *>
               (world.cur_map->cmap[dest[dim_y]][dest[dim_x]])) {
      io_battle(&world.pc, world.cur_map->cmap[dest[dim_y]][dest[dim_x]]);
      // Not actually moving, so set dest back to PC position
      dest[dim_x] = world.pc.pos[dim_x];
      dest[dim_y] = world.pc.pos[dim_y];
    }
  }
  
  if (move_cost[char_pc][world.cur_map->map[dest[dim_y]][dest[dim_x]]] ==
      INT_MAX) {
    return 1;
  }

  return 0;
}

void io_teleport_world(pair_t dest)
{
  int x, y;
  
  world.cur_map->cmap[world.pc.pos[dim_y]][world.pc.pos[dim_x]] = NULL;

  mvprintw(0, 0, "Enter x [-200, 200]: ");
  refresh();
  echo();
  curs_set(1);
  mvscanw(0, 21, (char *) "%d", &x);
  mvprintw(0, 0, "Enter y [-200, 200]:          ");
  refresh();
  mvscanw(0, 21, (char *) "%d", &y);
  refresh();
  noecho();
  curs_set(0);

  if (x < -200) {
    x = -200;
  }
  if (x > 200) {
    x = 200;
  }
  if (y < -200) {
    y = -200;
  }
  if (y > 200) {
    y = 200;
  }
  
  x += 200;
  y += 200;

  world.cur_idx[dim_x] = x;
  world.cur_idx[dim_y] = y;

  new_map(1);
  io_teleport_pc(dest);
}

void io_encounter_pokemon()
{
  Pokemon *p;
  
  int md = (abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)) +
            abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)));
  int minl, maxl;
  
  if (md <= 200) {
    minl = 1;
    maxl = md / 2;
  } else {
    minl = (md - 200) / 2;
    maxl = 100;
  }
  if (minl < 1) {
    minl = 1;
  }
  if (minl > 100) {
    minl = 100;
  }
  if (maxl < 1) {
    maxl = 1;
  }
  if (maxl > 100) {
    maxl = 100;
  }

  p = new Pokemon(rand() % (maxl - minl + 1) + minl);

  //  std::cerr << *p << std::endl << std::endl;

  io_queue_message("%s%s%s: HP:%d ATK:%d DEF:%d SPATK:%d SPDEF:%d SPEED:%d %s",
                   p->is_shiny() ? "*" : "", p->get_species(),
                   p->is_shiny() ? "*" : "", p->get_maxhp(), p->get_atk(),
                   p->get_def(), p->get_spatk(), p->get_spdef(),
                   p->get_speed(), p->get_gender_string());
  io_queue_message("%s's moves: %s %s", p->get_species(),
                   p->get_move(0), p->get_move(1));

  // Later on, don't delete if captured
  delete p;
}

void io_handle_input(pair_t dest)
{
  uint32_t turn_not_consumed;
  int key;

  do {
    switch (key = getch()) {
    case '7':
    case 'y':
    case KEY_HOME:
      turn_not_consumed = move_pc_dir(7, dest);
      break;
    case '8':
    case 'k':
    case KEY_UP:
      turn_not_consumed = move_pc_dir(8, dest);
      break;
    case '9':
    case 'u':
    case KEY_PPAGE:
      turn_not_consumed = move_pc_dir(9, dest);
      break;
    case '6':
    case 'l':
    case KEY_RIGHT:
      turn_not_consumed = move_pc_dir(6, dest);
      break;
    case '3':
    case 'n':
    case KEY_NPAGE:
      turn_not_consumed = move_pc_dir(3, dest);
      break;
    case '2':
    case 'j':
    case KEY_DOWN:
      turn_not_consumed = move_pc_dir(2, dest);
      break;
    case '1':
    case 'b':
    case KEY_END:
      turn_not_consumed = move_pc_dir(1, dest);
      break;
    case '4':
    case 'h':
    case KEY_LEFT:
      turn_not_consumed = move_pc_dir(4, dest);
      break;
    case '5':
    case ' ':
    case '.':
    case KEY_B2:
      dest[dim_y] = world.pc.pos[dim_y];
      dest[dim_x] = world.pc.pos[dim_x];
      turn_not_consumed = 0;
      break;
    case '>':
      turn_not_consumed = move_pc_dir('>', dest);
      break;
    case 'Q':
      dest[dim_y] = world.pc.pos[dim_y];
      dest[dim_x] = world.pc.pos[dim_x];
      world.quit = 1;
      turn_not_consumed = 0;
      break;
      break;
    case 't':
      /* Teleport the PC to a random place in the map.              */
      io_teleport_pc(dest);
      turn_not_consumed = 0;
      break;
    case 'T':
      /* Teleport the PC to any map in the world.                   */
      io_teleport_world(dest);
      turn_not_consumed = 0;
      break;
    case 'm':
      io_list_trainers();
      turn_not_consumed = 1;
      break;
    case 'q':
      /* Demonstrate use of the message queue.  You can use this for *
       * printf()-style debugging (though gdb is probably a better   *
       * option.  Not that it matters, but using this command will   *
       * waste a turn.  Set turn_not_consumed to 1 and you should be *
       * able to figure out why I did it that way.                   */
      io_queue_message("This is the first message.");
      io_queue_message("Since there are multiple messages, "
                       "you will see \"more\" prompts.");
      io_queue_message("You can use any key to advance through messages.");
      io_queue_message("Normal gameplay will not resume until the queue "
                       "is empty.");
      io_queue_message("Long lines will be truncated, not wrapped.");
      io_queue_message("io_queue_message() is variadic and handles "
                       "all printf() conversion specifiers.");
      io_queue_message("Did you see %s?", "what I did there");
      io_queue_message("When the last message is displayed, there will "
                       "be no \"more\" prompt.");
      io_queue_message("Have fun!  And happy printing!");
      io_queue_message("Oh!  And use 'Q' to quit!");

      dest[dim_y] = world.pc.pos[dim_y];
      dest[dim_x] = world.pc.pos[dim_x];
      turn_not_consumed = 0;
      break;
    default:
      /* Also not in the spec.  It's not always easy to figure out what *
       * key code corresponds with a given keystroke.  Print out any    *
       * unhandled key here.  Not only does it give a visual error      *
       * indicator, but it also gives an integer value that can be used *
       * for that key in this (or other) switch statements.  Printed in *
       * octal, with the leading zero, because ncurses.h lists codes in *
       * octal, thus allowing us to do reverse lookups.  If a key has a *
       * name defined in the header, you can use the name here, else    *
       * you can directly use the octal value.                          */
      mvprintw(0, 0, "Unbound key: %#o ", key);
      turn_not_consumed = 1;
    }
    refresh();
  } while (turn_not_consumed);
}
