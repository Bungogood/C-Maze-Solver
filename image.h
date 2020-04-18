#include <png.h>
#include <stdbool.h>

int width, height;
png_byte color_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
int number_of_passes;
png_bytep* row_pointers;

bool check (int r, int c);
void colour (int r, int c, int RGB);
void read_png_file(char* filename);
void write_png_file(char* filename);