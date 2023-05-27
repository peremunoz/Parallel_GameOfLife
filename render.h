#ifndef RENDER_H_
#define RENDER_H_

#include <stdbool.h>
#include "mpi.h"

extern bool Graphical_Mode;

void render_board(SDL_Renderer* renderer, board_t* board, 
                 unsigned char neighbors[D_COL_NUM][D_ROW_NUM]);

void mpi_render_board(SDL_Renderer* renderer, board_t* board,
                  unsigned char neighbors[D_COL_NUM][D_ROW_NUM],
                  int rank, MPI_Datatype rowType, int neighborsRank[2],
                  int firstRow, int lastRow, int size);

void render_running_state(SDL_Renderer *renderer, board_t *board);

void render_square(SDL_Renderer *renderer, int pos_x, int pos_y,
                  board_t* board);

void render_pause_state(SDL_Renderer *renderer, board_t *board);

void pause_square(SDL_Renderer *renderer, int pos_x, int pos_y,
                  board_t* board);

#endif // RENDER_H_

