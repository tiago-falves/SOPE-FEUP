#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "parser.h"
#include "register.h"

// ------------------- Argument Parsing ------------------------- //

struct Arguments args = { .max_depth = INT_MAX, .is_dir = true, .pathHasBar = false};

bool parseArguments(char *argv[], int argc)
{
    bool foundPath = false;
    char *flag;

    for (int i = 1; i < argc; i++)
    {
        //remove possible unnecessary slash bar at the end
        char savePath[MAX_FLAG_LEN];
        strcpy(savePath, argv[i]);
        if(argv[i][strlen(argv[i])-1] == '/') argv[i][strlen(argv[i]) - 1] = 0; 

        flag = strtok(argv[i], "=");
        int flagIndex = validFlag(flag);

        if (flagIndex != -1)
        {
            int number = -1;

            if (flagIndex == 5) // block size
            { 
                char * numberChar = strtok(NULL, "=");
                if(numberChar == NULL) {
                    printf("Invalid block size number argument (not provided)\n");
                    registerExit(-1);
                }
                else if (atoi(numberChar)) number = atoi(numberChar);
                else {
                    printf("Invalid block size (-B) number argument ('%s')\n", argv[i + 1]);
                    registerExit(-1);
                }
            }

            
            else if (flagIndex == 4) // block size short (-B) 
            {
                if(argv[i + 1] == NULL) {
                    printf("Invalid block size (-B) number argument\n");
                    registerExit(-1);
                }
                else if (atoi(argv[i+1])) {
                    number = atoi(argv[i+1]);
                }
                else {
                    printf("Invalid block size (-B) number argument ('%s')\n", argv[i + 1]);
                    registerExit(-1);
                }

                i++;
            }
            else if (flagIndex == 12) // max depth
            { 
                char * numberChar = strtok(NULL, "=");
                if(numberChar == NULL) {
                    printf("Invalid max depth number argument (not provided)\n");
                    registerExit(-1);
                }
                else if (numberChar[0] == '0' && strlen(numberChar) == 1) number = 0;
                else if (atoi(numberChar)) number = atoi(numberChar);
                else {
                    printf("Invalid max depth number argument ('%s')\n", numberChar);
                    registerExit(-1);
                }
            }

            if (!activateFlag(flag, number)) printf("Error in flag %s", flag);
        }
        else {

            int path_result = isPath(argv[i]);

            if (path_result == 0 || path_result == 2 ) {  // file or symbolic link
                foundPath = true;
                args.is_dir = false;
                strcpy(args.path, argv[i]);

                if(savePath[strlen(savePath)-1] == '/')
                    args.pathHasBar = true;
            }

            else if (path_result == 1) {  // directory
                foundPath = true;
                strcpy(args.path, argv[i]);

                if(savePath[strlen(savePath)-1] == '/')
                    args.pathHasBar = true;
            }

            else return false;
        }
    }

    if (!foundPath) {
        strcpy(args.path, ".");
    }

    return true;
}


int validFlag(char *flag)
{
    for (int i = 0; i < 13; i++)
    {
        if (!strcmp(flag, ARGUMENTS[i]))
        {
            return i; //Returns position in the ARGUMENTS array
        }
    }
    return -1;
}



bool activateFlag(char *flag, int number)
{
    int index = validFlag(flag);

    if (index == -1) return false;
    else if (!strcmp(ARGUMENTS[index], ALL_FLAG_SHORT) || !strcmp(ARGUMENTS[index], ALL_FLAG_LONG)) {
        args.all = true;
        return true;
    }
    else if (!strcmp(ARGUMENTS[index], BYTES_FLAG_SHORT) || !strcmp(ARGUMENTS[index], BYTES_FLAG_LONG)) {
        args.bytes = true;
        return true;
    }
    else if (!strcmp(ARGUMENTS[index], BLOCKSIZE_FLAG_SHORT) || !strcmp(ARGUMENTS[index], BLOCKSIZE_FLAG_LONG)) {
        args.block_size_flag = true;
        args.block_size = number;
        return true;
    }
    else if (!strcmp(ARGUMENTS[index], COUNTLINKS_FLAG_SHORT) || !strcmp(ARGUMENTS[index], COUNTLINKS_FLAG_LONG)) {
        args.countLinks = true;
        return true;
    }
    else if (!strcmp(ARGUMENTS[index], LINK_FLAG_SHORT) || !strcmp(ARGUMENTS[index], LINK_FLAG_LONG)) {
        args.deference = true;
        return true;
    }
    else if (!strcmp(ARGUMENTS[index], SEPARATEDIRS_FLAG_SHORT) || !strcmp(ARGUMENTS[index], SEPARATEDIRS_LONG)) {
        args.separateDirs = true;
        return true;
    }
    else if (!strcmp(ARGUMENTS[index], MAX_DEPTH_FLAG)) {
        args.max_depth = number;
        args.max_depth_flag = true;
        return true;
    }
    return false;
}


int isPath(const char *path)
{
    struct stat stbuf;
    if(stat(path, &stbuf) < 0) return -1;       // Invalid path
    else if(S_ISREG(stbuf.st_mode)) return 0;   // File
    else if(S_ISDIR(stbuf.st_mode)) return 1;   // Directory
    else if(S_ISLNK(stbuf.st_mode)) return 2;   // Symbolic link
    else return -1;
}

