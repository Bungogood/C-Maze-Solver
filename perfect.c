
//gcc perfect.c -o tmp -lpng
//./tmp images/hard.png solved/hard.png

#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <png.h>

typedef struct QueueItem {
    void* data;
    int priority;
    struct QueueItem* next;
    struct QueueItem* prev;
} QueueItem;

typedef struct Queue {
    struct QueueItem* head;
    struct QueueItem* tail;
} Queue;

typedef struct vertex {
    int r;
    int c;
    struct vertex* edges[4];
    struct vertex* prev;
    int dist;
    bool seen;
} vertex;

vertex* start;
vertex* end;

int width, height;
png_byte color_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
int number_of_passes;
png_bytep* row_pointers;

void error(const char * s, ...) {
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
}

vertex* newVertex (int r, int c) {
    vertex* tmp = malloc(sizeof (vertex));
    tmp->r = r;
    tmp->c = c;
    for (int i=0; i<4; i++) {
        tmp->edges[i] = NULL;
    }
    tmp->prev = NULL;
    tmp->dist = INT_MAX;
    tmp->seen = false;
    return tmp;
}

QueueItem* newItem (void* d, int p) {
    QueueItem* tmp = malloc(sizeof (QueueItem));
    tmp->data = d;
    tmp->priority = p;
    tmp->next = NULL;
    tmp->prev = NULL;
    return tmp;
}

Queue* newQueue () {
    Queue* tmp = malloc(sizeof (Queue));
    tmp->head = NULL;
    tmp->tail = NULL;
    return tmp;
}

void enqueue (Queue* Q, void* d, int p) {
    QueueItem* tmp = newItem(d, p);
    if (Q->head == NULL) {
        Q->head = tmp;
        Q->tail = tmp;
    } else if (Q->tail->priority <= p) {
        tmp->prev = Q->tail;
        Q->tail->next = tmp;
        Q->tail = tmp; 
    } else if (Q->tail->priority >= p) {
        tmp->next = Q->head;
        Q->head->prev = tmp;
        Q->head = tmp; 
    } else {
        QueueItem* cur = Q->tail;
        while (cur->prev != NULL && cur->prev->priority > p) {
            cur = cur->prev; 
        }
        cur->prev->next = tmp;
        tmp->prev = cur->prev;
        tmp->next = cur;
        cur->prev = tmp;
    }
}

void* dequeue (Queue* Q) {
    QueueItem* tmp = Q->head; 
    void* data = Q->head->data; 
    Q->head = Q->head->next;
    if (Q->head != NULL) {
        Q->head->prev = NULL;
    }
    free(tmp);
    return data;
}

bool check (int r, int c) {
    png_byte* row = row_pointers[r];
    png_byte* ptr = &(row[c*4]);
    return ptr[0] != 0x00 && ptr[1] != 0x00 && ptr[2] != 0x00;
}

void convert () {
    int r, c;
    int count = 0;
    vertex* prev;
    vertex* cur;
    vertex* prevline[width];
    

    for (c = 1; c < width-1; c++) {
        if (check(0, c)) {
            start = newVertex(0, c);
            count++;
            prevline[c] = start;
        } else {
            prevline[c] = NULL;
        }
    }

    for (r = 1; r < height-1; r++) {
        prev = NULL;
        for (c = 1; c < width-1; c++) {
            if (check(r, c)) {
                if (check(r+1, c) != check(r-1, c) || check(r, c+1) != check(r, c-1) || (check(r+1, c) && check(r-1, c) && check(r, c+1) && check(r, c-1))) {
                    cur = newVertex(r, c);
                    count++;
                    if (prev != NULL) {
                        cur->edges[3] = prev;
                        prev->edges[1] = cur;
                    }
                    if (prevline[c] != NULL) {
                        cur->edges[2] = prevline[c];
                        prevline[c]->edges[0] = cur;
                    }
                    prev = cur;
                    prevline[c] = cur;
                }
            } else {
                prev = NULL;
                prevline[c] = NULL;
            }
        }
    }

    for (c = 1; c < width-1; c++) {
        if (check(height-1, c)) {
            end = newVertex(height-1, c);
            count++;
            end->edges[2] = prevline[c];
            prevline[c]->edges[0] = end;
            break;
        }
    }
    printf("count: %d\n", count);
}

int distance (vertex* a, vertex* b) {
    return abs((a->r - b->r) + (a->c - b->c));
}

void colour (int r, int c, int RGB) {
    png_byte* row = row_pointers[r];
    png_byte* ptr = &(row[c*4]);
    ptr[0] = RGB >> 16 & 255;
    ptr[1] = RGB >> 8 & 255;
    ptr[2] = RGB & 255;
}

bool Dijkstra (vertex* start, vertex* end) {
    int alt;
    int count = 0;
    vertex* cur;
    vertex* edge;
    Queue* Q = newQueue();
    start->dist = 0;
    enqueue(Q, start, 0);
    while (Q->head != NULL) {
        cur = dequeue(Q);
        cur->seen = true;
        colour(cur->r, cur->c, 0x00FF00);
        count++;
        if (cur == end) {
            printf("count: %d\n", count);
            return true;
        }

        for (int i=0; i<4; i++) {
            edge = cur->edges[i];
            if (edge != NULL && !edge->seen) {
                alt = cur->dist + distance(cur, edge);
                if (alt < edge->dist) {
                    edge->prev = cur;
                    edge->dist = alt;
                    enqueue(Q, edge, alt);
                }
            }
        }
    }
    printf("count: %d\n", count);
    return false;
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

void drawpath (vertex* cur) {
    while (cur != NULL) {
        colour(cur->r, cur->c, 0xFF0000);
        cur = cur->prev;
    }
}

void drawfull (vertex* cur) {
    int ir;
    int ic;
    while (cur->prev != NULL) {
        if (cur->prev->r - cur->r < 0) {ir = -1;}
        else {ir = 1;}

        if (cur->prev->c - cur->c < 0) {ic = -1;}
        else {ic = 1;}

        for (int nr = cur->r; nr != cur->prev->r; nr+=ir) {
            colour(nr, cur->prev->c, 0xFF0000);
        }

        for (int nc = cur->c; nc != cur->prev->c; nc+=ic) {
            colour(cur->prev->r, nc, 0x0000FF);
        }
        cur = cur->prev;
    }
    colour(cur->r, cur->c, 0x00FF00);
}


void drawgradient (vertex* end, int s, int e) {
    vertex* cur = end;
    double count = 1;
    double inc[3];
    double col[3];
    int RGB = e;
    int ir;
    int ic;

    col[0] = e >> 16 & 255;
    col[1] = e >> 8 & 255;
    col[2] = e & 255;

    while (cur != NULL) {
        count++;
        cur = cur->prev;
    }

    inc[0] = ((s >> 16 & 255) - (e >> 16 & 255)) / count;
    inc[1] = ((s >> 8 & 255) - (e >> 8 & 255)) / count;
    inc[2] = ((s & 255) - (e & 255)) / count;

    cur = end;

    while (cur->prev != NULL) {
        if (cur->prev->r - cur->r < 0) {ir = -1;}
        else {ir = 1;}

        if (cur->prev->c - cur->c < 0) {ic = -1;}
        else {ic = 1;}

        for (int nr = cur->r; nr != cur->prev->r; nr+=ir) {
            colour(nr, cur->prev->c, RGB);
        }

        for (int nc = cur->c; nc != cur->prev->c; nc+=ic) {
            colour(cur->prev->r, nc, RGB);
        }
        col[0] += inc[0];
        col[1] += inc[1];
        col[2] += inc[2];
        RGB = (int)col[0] << 16 | (int)col[1] << 8 | (int)col[2];
        cur = cur->prev;
    }
    colour(cur->r, cur->c, s);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        error("Usage: program_name <file_in> <file_out>");
    }
    read_png_file(argv[1]); 
    convert();
    if (Dijkstra(start, end)) {
        printf("Completed\n");
        drawgradient(end, 0xFF0000, 0x0000FF);
    } else {
        printf("Failed\n");
    }
    write_png_file(argv[2]);
    return 0;
}