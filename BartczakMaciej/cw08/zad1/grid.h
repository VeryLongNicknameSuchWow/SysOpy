#pragma once

#include <stdbool.h>

typedef struct {
    char *src;
    char *dst;
    int cellNo;
} CellThreadArgs;

char *createGrid();

void destroyGrid(char *grid);

void drawGrid(const char *grid);

void initGrid(char *grid);

bool isAlive(int row, int col, const char *grid);

void update_grid(char *src, char *dst);