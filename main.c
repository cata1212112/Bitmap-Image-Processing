#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp_header.h"


#define max_instruction_size 100
#define max_instr_params 8
#define max_param_len 50
#define alignment 4
#define running 1
#define pixel_size 3

#define first 1
#define second 2
#define third 3
#define fourth 4
#define fifth 5
#define sixth 6
#define seventh 7

void Read_Image(char *file, img *Imagine) {
    FILE *input = fopen(file, "rb");
    fread(Imagine, sizeof(bmp_fileheader) + sizeof(bmp_infoheader), 1, input);

    fseek(input, (long)Imagine->header.imageDataOffset, SEEK_SET);

    int img_height = (int) Imagine->infoheader.height;
    int img_width = (int) Imagine->infoheader.width;
    int line_padding = ((pixel_size * img_width)%alignment);

    Imagine->imagine = (pixels**) calloc(img_height , sizeof(pixels*));
    if (Imagine->imagine == NULL) {
        free(Imagine->imagine);
        exit(0);
    }

    for (int i = 0; i < img_height; ++i) {
        Imagine->imagine[i] = (pixels*) calloc(img_width , sizeof(pixels));
        if (Imagine->imagine[i] == NULL) {
            for (int j = 0; j <= i; ++j) {
                free(Imagine->imagine[j]);
            }
            free(Imagine->imagine);
            exit(0);
        }
        for (int j = 0; j < img_width; ++j) {
            fread(&Imagine->imagine[i][j], 1, pixel_size, input);
        }
        if (line_padding > 0) {
            fseek(input, (long)(alignment - line_padding), SEEK_CUR);
        }
    }
    fclose(input);
}

void Write_Image(char *file, img Imagine) {
    FILE *output = fopen(file, "wb");
    int need_to_read = sizeof(bmp_fileheader) + sizeof(bmp_infoheader);
    fwrite(&Imagine, 1, sizeof(bmp_fileheader) + sizeof(bmp_infoheader), output);

    long long int byte_pos = ftell(output);
    unsigned int offset = Imagine.header.imageDataOffset;
    char byte = 0;
    for (int i = 0; i < offset - byte_pos; ++i) {
        fwrite(&byte, 1, 1, output);
    }

    int img_height = (int) Imagine.infoheader.height;
    int img_width = (int) Imagine.infoheader.width;
    int line_padding = ((pixel_size * img_width)%alignment);

    for (int i = 0; i < img_height; ++i) {
        for (int j = 0; j < img_width; ++j) {
            fwrite(&Imagine.imagine[i][j], pixel_size, 1, output);
        }
        if (line_padding > 0) {
            for (int j = 0; j < alignment - line_padding; ++j) {
                fwrite(&byte, 1, 1, output);
            }
        }
    }
    fclose(output);
}

void Insert_Image(char *file, img *to_edit, coord start) {
    img tmp;
    Read_Image(file, &tmp);

    unsigned int x_final = to_edit->infoheader.height;
    if (tmp.infoheader.height + start.x < to_edit->infoheader.height) {
        x_final = tmp.infoheader.height + start.x;
    }

    unsigned int y_final = to_edit->infoheader.width;
    if (tmp.infoheader.width + start.y < to_edit->infoheader.width) {
        y_final = tmp.infoheader.width + start.y;
    }

    unsigned int bytes_to_copy = 3 * (y_final - start.y);
    for (unsigned int i = start.x; i < x_final; ++i) {
        pixels *destination_address = &to_edit->imagine[i][0] + start.y;
        pixels *source_address = &tmp.imagine[i - start.x][0];
        memcpy(destination_address, source_address, bytes_to_copy);
    }

    for (unsigned int i = 0; i < tmp.infoheader.height; ++i) {
        free(tmp.imagine[i]);
    }
    free(tmp.imagine);
}


int get_y_given_x(int x, coord a, coord b) {
    return ((x - a.x) * (b.y - a.y) + a.y * (b.x - a.x)) / (b.x - a.x);
}

int get_x_given_y(int y, coord a, coord b) {
    return ((y - a.y) * (b.x - a.x) + a.x * (b.y - a.y)) / (b.y - a.y);
}


void draw_pixel_box(coord pixel, settings set, img *to_edit) {
    int x_start = pixel.x - set.line_width/2;
    int y_start = pixel.y - set.line_width/2;
    if (x_start < 0) x_start = 0;
    if (y_start < 0) y_start = 0;

    int x_final = pixel.x + set.line_width/2 + 1;
    int y_final = pixel.y + set.line_width/2 + 1;
    if (x_final >= to_edit->infoheader.height) x_final = (int) to_edit->infoheader.height;
    if (y_final >= to_edit->infoheader.width) y_final = (int) to_edit->infoheader.width;

    for (unsigned int i = x_start; i < x_final; ++i) {
        for (unsigned int j = y_start; j < y_final; ++j) {
            to_edit->imagine[i][j] = set.culoare;
        }
    }
}

void draw_line(coord start, coord final, settings set, img *to_edit) {
    int len_x = 0, len_y = 0;
    if (start.x < final.x) {
        len_x = final.x - start.x;
    } else {
        len_x = start.x - final.x;
    }

    if (start.y < final.y) {
        len_y = final.y - start.y;
    } else {
        len_y = start.y - final.y;
    }

    if (len_x >= len_y) {
        int min_lin = start.x;
        if (min_lin > final.x) {
            min_lin = final.x;
        }

        int max_lin = start.x + final.x - min_lin + 1;
        for (int i = min_lin; i < max_lin; ++i) {
            draw_pixel_box((coord) {i, get_y_given_x(i, start, final)}, set, to_edit);
        }

    } else {
        int min_col = start.y;
        if (min_col > final.y) {
            min_col = final.y;
        }

        int max_col = start.y + final.y - min_col + 1;
        for (int j = min_col; j < max_col; ++j) {
            draw_pixel_box((coord) {get_x_given_y(j, start, final), j}, set, to_edit);
        }
    }
}

void draw_rectangle(coord start, int width, int height, img *to_edit, settings set) {
    draw_line(start, (coord){start.x, start.y + width}, set, to_edit);
    draw_line(start, (coord){start.x + height, start.y}, set, to_edit);
    draw_line((coord) {start.x, start.y + width}, (coord){start.x + height, start.y + width}, set, to_edit);
    draw_line((coord) {start.x + height, start.y}, (coord){start.x + height, start.y + width}, set, to_edit);
}

void draw_triangle(coord p1, coord p2, coord p3, img *to_edit, settings set) {
    draw_line(p1, p2, set, to_edit);
    draw_line(p2, p3, set, to_edit);
    draw_line(p3, p1, set, to_edit);
}

int Is_In_Image(coord pos, img Imagine) {
    if (pos.x < 0 || pos.y < 0) return 0;
    if (pos.x >= Imagine.infoheader.height) return 0;
    if (pos.y >= Imagine.infoheader.width) return 0;
    return 1;
}

coord Get_Nord(coord pos) {
    return ((coord) {pos.x-1, pos.y});
}

coord Get_South(coord pos) {
    return ((coord) {pos.x+1, pos.y});
}
coord Get_West(coord pos) {
    return ((coord) {pos.x, pos.y - 1});
}

coord Get_East(coord pos) {
    return ((coord) {pos.x, pos.y + 1});
}


void Flood_fill(coord pos, img *to_edit, pixels *cul, pixels *start_cul) {
    if (!Is_In_Image(pos, *to_edit)) return;
    if (memcmp(&to_edit->imagine[pos.x][pos.y], cul, pixel_size) == 0) return;
    if (memcmp(&to_edit->imagine[pos.x][pos.y], start_cul, pixel_size) != 0) return;

    to_edit->imagine[pos.x][pos.y] = *cul;
    Flood_fill(Get_Nord(pos), to_edit, cul, start_cul);
    Flood_fill(Get_South(pos), to_edit, cul, start_cul);
    Flood_fill(Get_West(pos), to_edit, cul, start_cul);
    Flood_fill(Get_East(pos), to_edit, cul, start_cul);
}

int main() {
    img edited;
    int y = 0, x = 0;
    settings setari;
    setari.line_width = 1;
    char **parsed_instruction = (char **) calloc(max_instr_params, sizeof(char *));
    char *instruction = (char *) calloc(max_instruction_size, sizeof(char));
    for (int i = 0; i < max_instr_params; ++i) {
        parsed_instruction[i] = (char *) calloc(max_param_len, sizeof(char));
    }
    int cnt = 0;
    while (running) {
        fgets(instruction, max_instruction_size, stdin);

        char *p = strtok(instruction, " \n");
        cnt = 0;
        while (p != NULL) {
            strncpy(parsed_instruction[cnt], p, strlen(p)+1);
            cnt++;
            p = strtok(NULL, " \n");
        }

        if (strcmp(parsed_instruction[0], "quit") == 0) {
            break;
        } else if (strcmp(parsed_instruction[0], "save") == 0) {
            Write_Image(parsed_instruction[first], edited);
        } else if (strcmp(parsed_instruction[0], "edit") == 0) {
            Read_Image(parsed_instruction[first], &edited);
        } else if (strcmp(parsed_instruction[0], "insert") == 0) {
            y =  atoi(parsed_instruction[second]);
            x =  atoi(parsed_instruction[third]);
            Insert_Image(parsed_instruction[first], &edited, (coord){x, y});
        } else if (strcmp(parsed_instruction[0], "set") == 0) {
            if (strcmp(parsed_instruction[first], "draw_color") == 0) {
                unsigned char R =  atoi(parsed_instruction[second]);
                unsigned char G =  atoi(parsed_instruction[third]);
                unsigned char B =  atoi(parsed_instruction[fourth]);
				setari.culoare = (pixels){B, G, R};
            } else {
				setari.line_width = atoi(parsed_instruction[2]);
            }
        } else if (strcmp(parsed_instruction[0], "draw") == 0) {
            if (strcmp(parsed_instruction[1], "line") == 0) {
                int y1 = atoi(parsed_instruction[second]);
                int x1 = atoi(parsed_instruction[third]);

                int y2 = atoi(parsed_instruction[fourth]);
                int x2 = atoi(parsed_instruction[fifth]);

                draw_line((coord) {x1, y1}, (coord) {x2, y2}, setari, &edited);
            } else if (strcmp(parsed_instruction[first], "rectangle") == 0) {
                int y1 = atoi(parsed_instruction[second]);
                int x1 = atoi(parsed_instruction[third]);

                int width = atoi(parsed_instruction[fourth]);
                int height = atoi(parsed_instruction[fifth]);

                draw_rectangle((coord){x1, y1}, width, height, &edited, setari);
            } else {
                int y1 = atoi(parsed_instruction[second]);
                int x1 = atoi(parsed_instruction[third]);

                int y2 = atoi(parsed_instruction[fourth]);
                int x2 = atoi(parsed_instruction[fifth]);

                int y3 = atoi(parsed_instruction[sixth]);
                int x3 = atoi(parsed_instruction[seventh]);

                draw_triangle((coord){x1, y1}, (coord){x2, y2}, (coord){x3, y3}, &edited, setari);
            }
        } else {
            int y = atoi(parsed_instruction[first]);
            int x = atoi(parsed_instruction[second]);
            unsigned char B = edited.imagine[x][y].blue;
            unsigned char G = edited.imagine[x][y].green;
            unsigned char R = edited.imagine[x][y].red;
            pixels initial = (pixels) {B, G, R};
            Flood_fill((coord) {x, y}, &edited, &setari.culoare, &initial);
        }
    }
    free(instruction);
    for (int i = 0; i < max_instr_params; ++i) {
        free(parsed_instruction[i]);
    }
    free(parsed_instruction);
    return 0;
}
