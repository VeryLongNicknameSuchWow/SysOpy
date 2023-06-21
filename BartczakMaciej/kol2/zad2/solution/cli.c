#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define L_PYTAN 10

int main(void) {
    int status, gniazdo, i;
    struct sockaddr_in ser;
    char buf[200];
    char pytanie[] = "abccbahhff";

    gniazdo = socket(AF_INET, SOCK_STREAM, 0);
    if (gniazdo == -1) {
        printf("blad socket\n");
        return EXIT_FAILURE;
    }

    memset(&ser, 0, sizeof(ser));
    ser.sin_family = AF_INET;
    ser.sin_addr.s_addr = inet_addr("127.0.0.1");
    ser.sin_port = htons(8899);

    status = connect(gniazdo, (struct sockaddr *) &ser, sizeof(ser));
    if (status < 0) {
        printf("blad connect\n");
        return EXIT_FAILURE;
    }
    for (i = 0; i < L_PYTAN; i++) {
        status = write(gniazdo, pytanie + i, 1);
        if (status <= 0) {
            printf("blad write\n");
            return EXIT_FAILURE;
        }
        status = read(gniazdo, buf, sizeof(buf) - 1);
        if (status < 0) {
            printf("blad read\n");
            return EXIT_FAILURE;
        }
        buf[status] = '\0';
        printf("%s\n", buf);
    }
    printf("\n");

    close(gniazdo);
    printf("KONIEC DZIALANIA KLIENTA\n");
    return EXIT_SUCCESS;
}
