#define MESSAGE_LEN 256
#define NICKNAME_LEN 16

typedef enum {
    PING,
    USERNAME_TAKEN,
    SERVER_FULL,
    DISCONNECT,
    GET,
    LIST,
    TO_ONE,
    TO_ALL,
    STOP,
    CONNECT,
} MessageType;

typedef struct {
    MessageType type;
    char nickname[NICKNAME_LEN];
    char text[MESSAGE_LEN];
} message;
