#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *programCMD = "program";

void print_usage() {
    printf("Użycie:\n");
    printf("\t%s [nadawca | data]\n", programCMD);
    printf("\t%s <adresEmail> <tytul> <tresc>\n", programCMD);
}

int main(int argc, char *argv[]) {
    programCMD = argv[0];

    if (argc != 2 && argc != 4) {
        print_usage();
        return EXIT_FAILURE;
    }

    FILE *mail_pipe;
    char cmd[1024];

    if (argc == 2) {
        if (strcmp(argv[1], "nadawca") == 0) {
            strcpy(cmd, "mail -H | sort -k3,3");
        } else if (strcmp(argv[1], "data") == 0) {
            strcpy(cmd, "mail -H | sort -k5,5n -k6,6M -k7,7n -k8,8");
        } else {
            print_usage();
            return EXIT_FAILURE;
        }

        mail_pipe = popen(cmd, "r");
        if (mail_pipe == NULL) {
            perror("Nie można otworzyć potoku");
            return EXIT_FAILURE;
        }

        char buf[1024];
        while (fgets(buf, sizeof(buf), mail_pipe)) {
            printf("%s", buf);
        }

        pclose(mail_pipe);
    } else {
        snprintf(cmd, sizeof(cmd), "echo \"%s\" | mail -s \"%s\" \"%s\"", argv[3], argv[2], argv[1]);
        mail_pipe = popen(cmd, "w");
        if (mail_pipe == NULL) {
            perror("Nie można otworzyć potoku");
            return EXIT_FAILURE;
        }

        pclose(mail_pipe);
    }

    return EXIT_SUCCESS;
}
