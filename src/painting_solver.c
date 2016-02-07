#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(int * argc, char** argv) {
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

    fclose(input_file);
  }

  return 0;
}
