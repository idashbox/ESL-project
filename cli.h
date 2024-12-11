#ifndef CLI_H
#define CLI_H

#include <stdint.h>

void cli_init(void);
void cli_handle_command(const char *cmd);

#endif
