#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
  UNDEF,
  PAINT_SQUARE,
  PAINT_LINE,
  ERASE_CELL
} command_type_t;


typedef enum {
	CLEAR = 0,
	MUST_BE_PAINTED = 1,
  PAINTED_RIGHT = 2,
  PAINTED_WRONG = 3
} case_t;

static inline PAINT(case_t value) {
	switch(value) {
		case CLEAR:
			return PAINTED_WRONG;
		case MUST_BE_PAINTED:
			return PAINTED_RIGHT;
		default:
			return value;
	};
} 

static inline ERASE(case_t value) {
	switch(value) {
		case CLEAR:
		case MUST_BE_PAINTED:
			return value;
		case PAINTED_RIGHT:
			return MUST_BE_PAINTED;
		case PAINTED_WRONG:
			return CLEAR;
	};
}

typedef struct {
  command_type_t type;
  int coords[4];
} command_t;

struct command_cell_s {
  command_t cmd;
  struct command_cell_s* next;
};

typedef struct command_cell_s command_cell_t;

typedef struct {
  command_cell_t* first;
  command_cell_t* last;
  int length;
} command_list_t;

command_list_t* new_list() {
  command_list_t* result = (command_list_t*) malloc(sizeof(command_list_t));
  result->first = result->last = NULL;
  result->length = 0;
  return result;
}

void add_to_list(command_list_t* list, command_t cmd) {
  command_cell_t* new_cell = (command_cell_t*) malloc(sizeof(command_cell_t));

  new_cell->cmd = cmd;
  new_cell->next = NULL;
  
  if (list->last) {
    list->last->next = new_cell;
    list->last = new_cell;
  } else {
    list->first = new_cell;
    list->last = new_cell;
  };
  list->length += 1;
}

#define min(x, y) ((x < y) ? x : y)
#define max(x, y) ((x < y) ? y : x)

void execute_cmd(command_t cmd, unsigned char* array, int row_num, int col_num, int extended) {
  switch (cmd.type) {
    case PAINT_LINE: 
    {
      if (cmd.coords[0] == cmd.coords[2]) {
	  int row = cmd.coords[0];
	  int begin = min(cmd.coords[1], cmd.coords[3]);
	  int end   = max(cmd.coords[1], cmd.coords[3]);
	  int i;
	  for (i = begin; i <= end; ++i) array[row * col_num + i] = extended ? PAINT(array[row * col_num + i]) : 1;
      } else if (cmd.coords[1] == cmd.coords[3]) {
          int col = cmd.coords[1];
	  int begin = min(cmd.coords[0], cmd.coords[2]);
	  int end = max(cmd.coords[0], cmd.coords[2]);
	  int i;
	  for (i = begin; i <= end; ++i) array[i * col_num + col] = extended ? PAINT(array[i * col_num + col]) : 1;
      };
      break;
    }
    case PAINT_SQUARE:
    {
	    int row = cmd.coords[0];
      int col = cmd.coords[1];
      int   s = cmd.coords[2];
      int i, j;
      for (i = row - s; i <= row + s; ++i) 
	      for (j = col - s; j <= col + s; ++j)
		      array[i * col_num + j] = extended ? PAINT(array[i * col_num + j]) : 1;
    }
    case ERASE_CELL:
    {
	int row = cmd.coords[0];
        int col = cmd.coords[1];
        array[row * col_num + col] = extended ? ERASE(array[row * col_num + col]) : 0;
    }
    default:
      assert(0 && "unsupported command type in execute_cmd_list");
      break;
  };
}

void execute_cmd_list(command_list_t* cmd_list, unsigned char* array, int row_num, int col_num) {
  command_cell_t* current;
  for (current = cmd_list->first; current != NULL; current = current->next) {
    command_t cmd = current->cmd;
    execute_cmd(cmd, array, row_num, col_num, 0 /* not extended */);
  }
}

void print_cmd_list(command_list_t* cmd_list) {
  command_cell_t* current;
  for (current = cmd_list->first; current != NULL; current = current->next) {
    command_t cmd = current->cmd;
    switch (cmd.type) {
      case PAINT_LINE: 
      {
        printf("PAINT_LINE %d %d %d %d\n", cmd.coords[0], cmd.coords[1], cmd.coords[2], cmd.coords[3]);
        break;
      }
      default:
        assert(0 && "unsupported command type in execute_cmd_list");
        break;
    };
  }
}

void display_array(unsigned char* array, int row_num, int col_num) {
  int i, j;
  for (i = 0; i < row_num; ++i) {
    for (j = 0; j < col_num; ++j) printf("%c", array[i * col_num + j] ? '#' : '.');
    printf("\n");
  }
}


int main(int argc, char** argv) {
  const char*  usage = "Usage: ./painting_solver <input_file>\n";
  if (argc < 2) printf(usage);
  else {
    int row_num = -1, col_num = -1;

    FILE* input_file = fopen(argv[1], "r");
    int read_status = fscanf(input_file, "%d %d\n", &row_num, &col_num);
    assert((read_status == 2) && "failure while reading <row> <num> line");
    printf("file contains %d rows and %d columns\n", row_num, col_num);

    char* input_array = malloc(row_num * col_num * sizeof(unsigned char) + 1);
    int i;
    for (i = 0; i < row_num; ++i) {
      int read_status = fscanf(input_file, "%s\n", input_array + i * col_num);
      assert((read_status == 1) && "failure while reading row line");
    }
    printf("%d rows read\n", i);
    // cleaning input array
    for (i = 0; i < row_num * col_num; ++i) input_array[i] = input_array[i] == '#' ? MUST_BE_PAINTED : CLEAR;

    // command list
    command_list_t* cmd_list_p = new_list(); 
    for (i = 0; i < row_num; i++) {
      int j;
      for (j = 0; j < col_num; j++) {
        for (;j < col_num && input_array[i*col_num+j] != MUST_BE_PAINTED; ++j);
        if (j >= col_num) break;
        int begin_row = j;
        for (;j < col_num && input_array[i*col_num+j] == MUST_BE_PAINTED; ++j);
        int end_row = j - 1;
        int row_length = end_row - begin_row + 1;
        command_t cmd;
        int k;
        for (k = i; k < row_num && input_array[k * col_num + begin_row] == MUST_BE_PAINTED; ++k);
        int end_col = k - 1;
        int col_length = end_col - i + 1;
        if (col_length > row_length) {
          cmd.type = PAINT_LINE;
          cmd.coords[0] = i;
          cmd.coords[1] = begin_row;
          cmd.coords[2] = end_col;
          cmd.coords[3] = begin_row;
          j = begin_row;
        } else {
          cmd.type = PAINT_LINE;
          cmd.coords[0] = i;
          cmd.coords[1] = begin_row;
          cmd.coords[2] = i;
          cmd.coords[3] = end_row;
        }



        execute_cmd(cmd, input_array, row_num, col_num, 1 /* extended */);

        add_to_list(cmd_list_p, cmd);
      }
    }
    // checking
    unsigned char* reconstructed_array = calloc(row_num * col_num, sizeof(unsigned char));
    print_cmd_list(cmd_list_p);
    execute_cmd_list(cmd_list_p, reconstructed_array, row_num, col_num);
    display_array(reconstructed_array, row_num, col_num);

    printf("solved in %d commands\n", cmd_list_p->length);

    fclose(input_file);
  }

  return 0;
}
