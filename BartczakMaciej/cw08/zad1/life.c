#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <stdbool.h>
#include "grid.h"
#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(NULL));
    setlocale(LC_CTYPE, "");
    initscr(); // Start curses mode

    char *foreground = createGrid();
    char *background = createGrid();
    char *tmp;

    initGrid(foreground);

    while (true) {
        drawGrid(foreground);
        usleep(500 * 1000);

        // Step simulation
        update_grid(foreground, background);
        tmp = foreground;
        foreground = background;
        background = tmp;
    }

    endwin(); // End curses mode
    destroyGrid(foreground);
    destroyGrid(background);

    return 0;
}
