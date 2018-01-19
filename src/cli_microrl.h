#ifndef CLI_MICRORL_H
#define CLI_MICRORL_H

typedef struct card {
    char *UID;
    char *size;
    char *name;
    struct card *next;
} Card_t;

int cli_execute(int argc, const char *const *argv);
extern Card_t *header;

#endif
