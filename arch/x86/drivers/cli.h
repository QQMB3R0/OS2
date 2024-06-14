#ifndef CLI_H
#define CLI_H
#include "../inc/types.h"
#include "../io/GlobalObj.h"
#include "../io/Keyboard.h"
#include "../memory/heap.h"
#include "../memory/memory.h"
#include "../fileSystem/Fat32.h"


#define COMMAND_IN_DIR                          "cd"
#define COMMAND_OUT_DIR                         ".."
#define COMMAND_LIST_DIR                        "ls"
#define COMMAND_DELETE_CONTENT                  "rm"
#define COMMAND_MAKE_FILE                       "mkfile"
#define COMMAND_MAKE_DIR                        "mkdir"
#define BUFF_LEN 24

class cli
{
public:
    void init_cli();
    void cli_statr_screen();
    void execute_command(char* Command);
    char* get_path();
};
#endif