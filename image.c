#include "image.h"

bool check (int r, int c) {
    png_byte* row = row_pointers[r];
    png_byte* ptr = &(row[c*4]);
    return ptr[0] != 0x00 && ptr[1] != 0x00 && ptr[2] != 0x00;
}

void colour (int r, int c, int RGB) {
    png_byte* row = row_pointers[r];
    png_byte* ptr = &(row[c*4]);
    ptr[0] = RGB >> 16 & 255;
    ptr[1] = RGB >> 8 & 255;
    ptr[2] = RGB & 255;
}

void read_png_file(char* filename) {
    char header[8];
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        error("File %s could not be opened for reading", filename);
    }
    fread(header, 1, 8, fp);
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);
    row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for (int r=0; r<height; r++) {
        row_pointers[r] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
    }
    png_read_image(png_ptr, row_pointers);
    fclose(fp);
}


void write_png_file(char* filename) {
        FILE *fp = fopen(filename, "wb");
        if (!fp) {
            error("File %s could not be opened for writing", filename);
        }
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        info_ptr = png_create_info_struct(png_ptr);
        png_init_io(png_ptr, fp);
        png_set_IHDR(png_ptr, info_ptr, width, height,
                     bit_depth, color_type, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
        png_write_info(png_ptr, info_ptr);
        png_write_image(png_ptr, row_pointers);
        png_write_end(png_ptr, NULL);
        for (int r=0; r<height; r++) {
                free(row_pointers[r]);
        }
        free(row_pointers);
        fclose(fp);
}