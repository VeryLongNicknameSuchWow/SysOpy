#include "grid.h"
#include <stdlib.h>
#include <ncurses.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

#define GRID_WIDTH 40
#define GRID_HEIGHT 20
#define THREAD_COUNT GRID_WIDTH * GRID_HEIGHT

pthread_t threads[THREAD_COUNT];
CellThreadArgs gridArgs[THREAD_COUNT];

char *createGrid() {
    return calloc(GRID_WIDTH * GRID_HEIGHT, sizeof(char));
}

void destroyGrid(char *grid) {
    free(grid);
}

void drawGrid(const char *grid) {
    for (int row = 0; row < GRID_HEIGHT; ++row) {
        // Two characters for more uniform spaces (vertical vs horizontal)
        for (int col = 0; col < GRID_WIDTH; ++col) {
            if (grid[row * GRID_WIDTH + col]) {
                mvprintw(row, col * 2, "■");
                mvprintw(row, col * 2 + 1, " ");
            } else {
                mvprintw(row, col * 2, " ");
                mvprintw(row, col * 2 + 1, " ");
            }
        }
    }

    refresh();
}

void initGrid(char *grid) {
    for (int i = 0; i < GRID_WIDTH * GRID_HEIGHT; ++i) {
        grid[i] = rand() % 6 == 0;
    }
}

bool isAlive(int row, int col, const char *grid) {
    int aliveNeighbours = 0;
    for (int dRow = -1; dRow <= 1; dRow++) {
        for (int dCol = -1; dCol <= 1; dCol++) {
            if (dRow == 0 && dCol == 0) {
                continue;
            }
            int neighbourRow = row + dRow;
            int neighbourCol = col + dCol;
            if (neighbourRow < 0 || neighbourRow >= GRID_HEIGHT || neighbourCol < 0 || neighbourCol >= GRID_WIDTH) {
                continue;
            }
            if (grid[GRID_WIDTH * neighbourRow + neighbourCol]) {
                aliveNeighbours++;
            }
        }
    }

    if (grid[row * GRID_WIDTH + col]) {
        return aliveNeighbours == 2 || aliveNeighbours == 3;
    }
    return aliveNeighbours == 3;
}

void ignoreSignal(int _) {}

void *cellThreadRoutine(void *args) {
    CellThreadArgs *threadArgs = (CellThreadArgs *) args;
    int cellNo = threadArgs->cellNo;

    while (1) {
        threadArgs->dst[cellNo] = (char) isAlive(cellNo / GRID_WIDTH, cellNo % GRID_WIDTH, threadArgs->src);
        pause();

        char *tmp = threadArgs->src;
        threadArgs->src = threadArgs->dst;
        threadArgs->dst = tmp;
    }

    return NULL;
}

void update_grid(char *src, char *dst) {
    signal(SIGUSR1, ignoreSignal);

    for (int i = 0; i < THREAD_COUNT; i++) {
        CellThreadArgs *args = gridArgs + i;
        args->src = src;
        args->dst = dst;
        args->cellNo = i;

        pthread_create(threads + i, NULL, cellThreadRoutine, (void *) args);
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_kill(threads[i], SIGUSR1);
    }
}