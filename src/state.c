#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_state_t *state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t *state, unsigned int snum);
static char next_square(game_state_t *state, unsigned int snum);
static void update_tail(game_state_t *state, unsigned int snum);
static void update_head(game_state_t *state, unsigned int snum);

/* Task 1 */
game_state_t *create_default_state()
{
  // TODO: Implement this function.
  unsigned int rows = 18;
  unsigned int cols = 20;

  // malloc the state
  game_state_t *state = malloc(sizeof(game_state_t));
  if (state == NULL)
  {
    printf("malloc game_state fail");
    return NULL;
  }

  // malloc the rows
  state->num_rows = rows;
  state->board = (char **)malloc(rows * sizeof(char *));
  if (state->board == NULL)
  {
    printf("malloc fail");
    return NULL;
  }

  // malloc the cols
  for (size_t i = 0; i < rows; i++)
  {
    state->board[i] = (char *)malloc((cols + 1) * sizeof(char));
    if (state->board[i] == NULL)
    {
      printf("malloc cols fail");
      return NULL;
    }
    if (i == 0 || i == rows - 1)
    {
      strcpy(state->board[i], "####################\n");
    }
    else if (i == 2)
    {
      strcpy(state->board[i], "# d>D    *         #\n");
    }
    else
    {
      strcpy(state->board[i], "#                  #\n");
    }
  }

  state->num_snakes = 1;

  // malloc the snakes arrays
  state->snakes = (snake_t *)malloc(sizeof(snake_t));
  if (state->snakes == NULL)
  {
    printf("malloc snakes fail");
    return NULL;
  }

  state->snakes->head_row = 2;
  state->snakes->head_col = 4;
  state->snakes->tail_row = 2;
  state->snakes->tail_col = 2;
  state->snakes->live = true;
  return state;
}

/* Task 2 */
void free_state(game_state_t *state)
{
  // TODO: Implement this function.
  // free snakes
  free(state->snakes);
  for (size_t i = 0; i < state->num_rows; i++)
  {
    // free every line of board
    free(state->board[i]);
  }

  // free board
  free(state->board);

  // free state itself
  free(state);
  return;
}

/* Task 3 */
void print_board(game_state_t *state, FILE *fp)
{
  // TODO: Implement this function.
  for (size_t i = 0; i < state->num_rows; i++)
  {
    fprintf(fp, "%s", state->board[i]);
  }
  return;
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t *state, char *filename)
{
  FILE *f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t *state, unsigned int row, unsigned int col) { return state->board[row][col]; }

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t *state, unsigned int row, unsigned int col, char ch)
{
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c)
{
  // TODO: Implement this function.

  return c == 'w' || c == 'a' || c == 's' || c == 'd';
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c)
{
  // TODO: Implement this function.
  return c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x';
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c)
{
  // TODO: Implement this function.
  return is_head(c) || is_tail(c) || c == '^' || c == '<' || c == 'v' || c == '>';
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c)
{
  // TODO: Implement this function.
  char match;
  switch (c)
  {
  case '^':
    match = 'w';
    break;
  case '<':
    match = 'a';
    break;
  case 'v':
    match = 's';
    break;
  case '>':
    match = 'd';
    break;
  }
  return match;
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c)
{
  // TODO: Implement this function.
  char match;
  switch (c)
  {
  case 'W':
    match = '^';
    break;
  case 'A':
    match = '<';
    break;
  case 'S':
    match = 'v';
    break;
  case 'D':
    match = '>';
    break;
  }
  return match;
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c)
{
  // TODO: Implement this function.
  if (c == 'v' || c == 's' || c == 'S')
  {
    cur_row = cur_row + 1;
  }

  if (c == '^' || c == 'w' || c == 'W')
  {
    cur_row = cur_row - 1;
  }

  return cur_row;
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c)
{
  // TODO: Implement this function.
  if (c == '>' || c == 'd' || c == 'D')
  {
    cur_col = cur_col + 1;
  }

  if (c == '<' || c == 'a' || c == 'A')
  {
    cur_col = cur_col - 1;
  }
  return cur_col;
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t *state, unsigned int snum)
{
  // TODO: Implement this function.
  snake_t snake = state->snakes[snum];
  unsigned int next_row, next_col;
  next_row = get_next_row(snake.head_row, get_board_at(state, snake.head_row, snake.head_col));
  next_col = get_next_col(snake.head_col, get_board_at(state, snake.head_row, snake.head_col));
  return get_board_at(state, next_row, next_col);
}

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_state_t *state, unsigned int snum)
{
  // TODO: Implement this function.
  snake_t snake = state->snakes[snum];
  unsigned int row = snake.head_row, col = snake.head_col;
  // store the head's character
  char head = get_board_at(state, row, col);

  // get coresponding body's character to head
  char body_match_head = head_to_body(head);

  //  change head's character to coresponding body's character
  set_board_at(state, row, col, body_match_head);

  // update the row and col to next position that head will move to
  row = get_next_row(row, get_board_at(state, snake.head_row, snake.head_col));
  col = get_next_col(col, get_board_at(state, snake.head_row, snake.head_col));
  set_board_at(state, row, col, head);

  // update the place of snake's head
  state->snakes[snum].head_row = row;
  state->snakes[snum].head_col = col;
  return;
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t *state, unsigned int snum)
{
  // TODO: Implement this function.
  snake_t snake = state->snakes[snum];
  unsigned int row = snake.tail_row, col = snake.tail_col;
  // store the head's character
  char tail = get_board_at(state, row, col);

  //  change tail's character to ' '
  set_board_at(state, row, col, ' ');

  // update the row and col to next position that head will move to
  row = get_next_row(row, tail);
  col = get_next_col(col, tail);

  // get body
  char body = get_board_at(state, row, col);

  // update tail's character to match the body
  tail = body_to_tail(body);
  set_board_at(state, row, col, tail);

  // update the place of snake's tail
  state->snakes[snum].tail_row = row;
  state->snakes[snum].tail_col = col;
  return;
}

/* Task 4.5 */
void update_state(game_state_t *state, int (*add_food)(game_state_t *state))
{
  // TODO: Implement this function.
  for (unsigned int i = 0; i < state->num_snakes; ++i)
  {
    snake_t snake = state->snakes[i];
    char head = get_board_at(state, snake.head_row, snake.head_col);
    unsigned int next_row = get_next_row(snake.head_row, head);
    unsigned int next_col = get_next_col(snake.head_col, head);
    char next_char = get_board_at(state, next_row, next_col);
    /* next character
      ' ': safe
      '*': food
      other: '#' or snake'body
    */
    switch (next_char)
    {
    case '*':
      update_head(state, i);
      add_food(state);
      break;
    case ' ':
      update_head(state, i);
      update_tail(state, i);
      break;
    default:
      state->snakes[i].live = false;
      set_board_at(state, snake.head_row, snake.head_col, 'x');
      break;
    }
  }
  return;
}

/* Task 5.1 */
char *read_line(FILE *fp)
{
  // TODO: Implement this function.
  const long unsigned int buffer_size = 1024;
  char *buffer = (char *)malloc(buffer_size * sizeof(char));
  if (buffer == NULL)
  {
    printf("malloc buffer fail");
    return NULL;
  }
  int size = 1024;
  if (fgets(buffer, size, fp) == NULL)
  {
    // If fgets fails, free the buffer and return NULL
    free(buffer);
    return NULL;
  }
  size_t len = strlen(buffer);
  buffer = (char *)realloc(buffer, len + 1);
  return buffer;
}

/* Task 5.2 */
game_state_t *load_board(FILE *fp)
{
  // TODO: Implement this function.
  game_state_t * state =(game_state_t*) malloc(sizeof(game_state_t));
  if (state == NULL)
  {
    return NULL;
  }

  const unsigned int rows = 100;
  state->board = (char **) malloc(rows * sizeof(char*));
  if (state->board == NULL)
  {
    return NULL;
  }
  unsigned int row_index = 0;
  while ((state->board[row_index] = read_line(fp)) != NULL)
  {
    row_index = row_index + 1;
  }
  state->num_rows = row_index;
  state->board = (char **) realloc(state->board, row_index * sizeof(char*));
  
  state->num_snakes = 0;
  state->snakes = NULL;
  return state;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t *state, unsigned int snum)
{
  // TODO: Implement this function.
  snake_t snake = state->snakes[snum];
  unsigned int row = snake.tail_row;
  unsigned int col = snake.tail_col;

  // get tail character
  char ch = get_board_at(state, row, col);

  // get next body block connect to tail
  row = get_next_row(row, ch);
  col = get_next_col(col, ch);
  ch = get_board_at(state, row, col);

  // loop until head
  while (!is_head(ch))
  {
    row = get_next_row(row, ch);
    col = get_next_col(col, ch);
    ch = get_board_at(state, row, col);
  }
  state->snakes[snum].head_row = row;
  state->snakes[snum].head_col = col;
  return;
}

/* Task 6.2 */
game_state_t *initialize_snakes(game_state_t *state)
{
  // TODO: Implement this function.
  unsigned int num = 10;
  state->snakes = (snake_t *)malloc(num * sizeof(snake_t));
  // traverse the board to find snakes
  for (unsigned int i = 0; i < state->num_rows; i++)
  {
    for (unsigned int j = 0; j < strlen(state->board[i]); j++)
    {
      char ch = get_board_at(state, i, j);

      // if find a tail
      if(is_tail(ch)) {
        // if snakes larger than array space, relloc
        if (state->num_snakes >= num)
        {
          num *= 2;
          state->snakes = (snake_t *)realloc(state->snakes, num * sizeof(snake_t));
        }
        // set up snake
        state->snakes[state->num_snakes].tail_row = i;
        state->snakes[state->num_snakes].tail_col = j;
        state->snakes[state->num_snakes].live = true;
        find_head(state, state->num_snakes);
        
        // add one to num_snake
        state->num_snakes++;
      }
    }
    
  }

  state->snakes = (snake_t *)realloc(state->snakes, state->num_snakes * sizeof(snake_t));
  
  return state;
}
