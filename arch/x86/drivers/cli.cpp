#include "cli.h"
char* current_path = "HOME";
int exit = 1;
int current_command = 0;

void cli::init_cli()
{
    Keyboard k;
    display << ">"<<current_path<<'\t';    
    char input[BUFF_LEN + 1] = {'\0'}; 
    while (1)
    {
        uint32 i;
        for ( i = 0; i < BUFF_LEN; i++)
        {
            char c = k.getChar();
            input[i] = c;
            if(c == '\n') break;
        }
        int last_char = strlen(input) - 1;
        input[last_char] = '\0';
        execute_command(input);
    }
}
int scan_command(char *buffer, char **comm, char **fname) {
    char *buffscan = buffer;
    if(!*buffscan) {
        // There's nothing in the buffer.
        return 0;
    }

    // skip any whitespace
    while(*buffscan && *buffscan == ' ') {
        buffscan++;
    }

    // Make sure there's something afterwards
    if(!*buffscan) {
        return 0;
    }

    // Point comm at the first non-whitespace character.
    *comm = buffscan;

    // Find a space.
    while(*buffscan && *buffscan != ' ') {
        buffscan++;
    }

    if(!*buffscan) {
        // There's no more in the string
        return 1;
    }
    // Terminate the string.
    *buffscan = 0;
    buffscan++;

    // skip any whitespace
    while(*buffscan && *buffscan == ' ') {
        buffscan++;
    }

    // If there's no more string left, return 1.
    if(!*buffscan) {
        return 1;
    }

    *fname = buffscan;

    // Chop any space off the end.
    while(*buffscan && *buffscan != ' ') {
        buffscan++;
    }
    *buffscan = 0;

    return 2;
}

void cli::execute_command(char *Command)
{
    Fat32 fs;
    dir_entry_t* dir;;

    if (Command == NULL) return;
    if (strlen(Command) <= 0) return;
    char *command = NULL;
    char *filename = NULL;
    int scanned = scan_command(Command, &command, &filename);
    if (strcmp(command,COMMAND_MAKE_DIR)==0)
    {
        display << "\nyour command: "<<command;
        display << "\nyour filename: "<<filename<<"\n";
        //Content* content = fs.FAT_create_content(filename,true,"\0");
        //fs.setFile(current_path,content);
    }
    if (strcmp(command,COMMAND_OUT_DIR)==0)
    {
        display << "\nyour command: "<<command;
        display << "\nyour filename: "<<filename<<"\n";
    }
    if (strcmp(command,COMMAND_DELETE_CONTENT)==0)
    {
        display << "\nyour command: "<<command;
        display << "\nyour filename: "<<filename<<"\n";
    }
    if (strcmp(command,COMMAND_MAKE_FILE)==0)
    {
        display << "\nyour command: "<<command;
        display << "\nyour filename: "<<filename<<"\n";
    }
    if (strcmp(command,COMMAND_IN_DIR)==0)
    {
        display << "\nyour command: "<<command;
        display << "\nyour filename: "<<filename<<"\n";
    }
    if (strcmp(command,COMMAND_LIST_DIR)==0)
    {
        display << "\nyour command: "<<command;
        display << "\nyour filename: "<<filename<<"\n";
    }
    

}