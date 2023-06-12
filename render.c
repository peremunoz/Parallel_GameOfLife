#include <SDL2/SDL.h>

#include "./game.h"
#include "./render.h"
#include "./logic.h"

bool Graphical_Mode = false;

void render_board(SDL_Renderer* renderer, board_t* board,
                  unsigned char neighbors[D_COL_NUM][D_ROW_NUM])
{
  switch(board->game_state) {
    case RUNNING_STATE:
      if (Graphical_Mode)
        render_running_state(renderer, board);
      count_neighbors(board, neighbors);
      evolve(board, neighbors);
      break;
    case PAUSE_STATE:
      if (Graphical_Mode)
        render_pause_state(renderer, board);
      break;
    default: {}
  }
}

void mpi_render_board(SDL_Renderer* renderer, board_t* board,
                  unsigned char neighbors[D_COL_NUM][D_ROW_NUM],
                  int rank, MPI_Datatype rowType, int neighborsRank[2],
                  int firstRow, int lastRow, int size, int iteration)
{
  switch(board->game_state) {
    case RUNNING_STATE:

      if ((iteration == 0) || (rank != 0 && iteration > 0)) {
        // The rank 0 process doesn't need to receive the neighbors cells, because it already has them from the gather operation (except for the iteration 0)
      
        // Receive the neighbors cells from the other processes

        // Receive the top adjacent row
        MPI_Request topRowRequest;
        int topRowToReceive = firstRow == 0 ? board->ROW_NUM - 1 : firstRow - 1;
        MPI_Irecv(&board->cell_state[topRowToReceive][0], 1, rowType, neighborsRank[0], UP_TO_DOWN_TAG, MPI_COMM_WORLD, &topRowRequest);

        // Receive the bottom adjacent row
        MPI_Request bottomRowRequest;
        int bottomRowToReceive = lastRow == board->ROW_NUM - 1 ? 0 : lastRow + 1;
        MPI_Irecv(&board->cell_state[bottomRowToReceive][0], 1, rowType, neighborsRank[1], DOWN_TO_UP_TAG, MPI_COMM_WORLD, &bottomRowRequest);

        // Wait for the receive operations to complete
        MPI_Wait(&topRowRequest, MPI_STATUS_IGNORE);
        MPI_Wait(&bottomRowRequest, MPI_STATUS_IGNORE);

        if (rank==1) {
          printf("Row %i received from rank %d: ", bottomRowToReceive, neighborsRank[1]);
          for (int i = 0; i < board->COL_NUM; i++) {
            printf("%u", board->cell_state[bottomRowToReceive][i]);
          }
          printf("\n");
          printf("Row %i received from rank %d: ", topRowToReceive, neighborsRank[0]);
          for (int i = 0; i < board->COL_NUM; i++) {
            printf("%u", board->cell_state[topRowToReceive][i]);
          }
          fflush(stdout);
        }

      }

      if (Graphical_Mode && rank == 0)
        render_running_state(renderer, board);

      count_neighbors(board, neighbors);
      evolve(board, neighbors);

      // Send the neighbors cells to the other processes

      // As the rank 0 process doesn't need to receive the neighbors cells, rank 1 can avoid this send operation
      if (rank != 1) {
        MPI_Request topRowSendRequest;
        // Send the first row to the top adjacent process
        MPI_Isend(&board->cell_state[firstRow][0], 1, rowType, neighborsRank[0], DOWN_TO_UP_TAG, MPI_COMM_WORLD, &topRowSendRequest);
      }
      
      // Same for the last process, which doesn't need to send the bottom row to the first process (rank 0)
      if (rank != size - 1) {
        MPI_Request bottomRowSendRequest;
        // Send the last row to the bottom adjacent process
        MPI_Isend(&board->cell_state[lastRow][0], 1, rowType, neighborsRank[1], UP_TO_DOWN_TAG, MPI_COMM_WORLD, &bottomRowSendRequest);
      }

      break;
    case PAUSE_STATE:
      if (Graphical_Mode && rank == 0)
        render_pause_state(renderer, board);
      break;
    default: {}
  }
}

void render_running_state(SDL_Renderer *renderer, board_t *board)
{
  int pos_x = 0;
  int pos_y = 0;

  for (int i = 0; i < board->COL_NUM; i++) {
    for (int j = 0; j < board->ROW_NUM; j++) {
      pos_x = i * board->CELL_WIDTH;
      pos_y = j * board->CELL_HEIGHT;
      if (board->cell_state[i][j] == ALIVE)
        render_square(renderer, pos_x, pos_y, board);
    }
  }
}

void render_pause_state(SDL_Renderer *renderer, board_t *board)
{
  int pos_x = 0;
  int pos_y = 0;

  for (int i = 0; i < board->COL_NUM; i++) {
    for (int j = 0; j < board->ROW_NUM; j++) {
      pos_x = i * board->CELL_WIDTH;
      pos_y = j * board->CELL_HEIGHT;
      if (board->cell_state[i][j] == ALIVE)
        pause_square(renderer, pos_x, pos_y, board);
    }
  }
}

void render_square(SDL_Renderer *renderer, int pos_x, int pos_y, board_t* board)
{
  SDL_Rect cell;
  cell.x = pos_x;
  cell.y = pos_y;
  cell.w = board->CELL_WIDTH - 1;
  cell.h = board->CELL_HEIGHT - 1;
  SDL_SetRenderDrawColor(renderer, 142, 192, 124, 1);
  SDL_RenderFillRect(renderer, &cell);
}

void pause_square(SDL_Renderer *renderer, int pos_x, int pos_y, board_t* board)
{
  SDL_Rect cell;
  cell.x = pos_x;
  cell.y = pos_y;
  cell.w = board->CELL_WIDTH - 1;
  cell.h = board->CELL_HEIGHT - 1;
  SDL_SetRenderDrawColor(renderer, 146, 131, 116, 1);
  SDL_RenderFillRect(renderer, &cell);
}


