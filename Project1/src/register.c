#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <string.h>
#include "register.h"

int registersFileDescriptor;

struct tms t;
clock_t start, end;
long ticks;
    

bool createRegistersFile()
{
    char * logFilename = getenv("LOG_FILENAME");
    if(logFilename==NULL){
        logFilename = "registers.txt";
    }
    
    registersFileDescriptor = open(logFilename, O_WRONLY | O_TRUNC | O_CREAT, 0644);

    if (registersFileDescriptor < 0)
    {
        printf("Error opening file");
        registerExit(1);
    }

    ticks = sysconf(_SC_CLK_TCK);
    start = times(&t);
    return true;
}

void setInitialTime()
{

}

struct Register createRegister(int action)
{
    struct Register reg;
    reg.action = action;
    reg.instant = times(&t) - start;
    reg.pid = getpid();
    return reg;
}

void writeRegister(const struct Register *reg)
{
    char regString[MAX_REG_LEN];
    sprintf(regString, "%.2ld - %.8d - %s - %s\n", reg->instant/ticks, reg->pid, actionString(reg->action), reg->info);
    write(registersFileDescriptor, regString, strlen(regString));
}

void registerCreate(char *argv[], int argc) {
    struct Register reg =  createRegister(CREATE);
    
    strcpy(reg.info, "\0");
    for(int i = 0; i < argc; i++){
		strcat(reg.info, argv[i]);
        strcat(reg.info," ");
    }

    writeRegister(&reg);
}

void registerExit(int exitStatus)
{
    struct Register reg = createRegister(EXIT);
    sprintf(reg.info, "%d", exitStatus);
    writeRegister(&reg);
    exit(exitStatus);
}

void registerRecSignal(int signal)
{
    struct Register reg = createRegister(RECV_SIGNAL);
    sprintf(reg.info, "%s", signalString(signal));
    writeRegister(&reg);
}

void registerSendSignal(pid_t pid, int signal)
{
    struct Register reg = createRegister(SEND_SIGNAL);
    sprintf(reg.info, "%s %d", signalString(signal), pid);
    writeRegister(&reg);
}

void registerRecPipe(int size)
{
    struct Register reg = createRegister(RECV_PIPE);
    sprintf(reg.info, "%d",size);
    writeRegister(&reg);
}

void registerSendPipe(int size)
{
    struct Register reg = createRegister(SEND_PIPE);
    sprintf(reg.info, "%d",size);
    writeRegister(&reg);
}

void registerEntry(long bytes, char * path)
{
    // número de bytes (ou blocos) seguido do caminho.
    struct Register reg = createRegister(ENTRY);
    sprintf(reg.info, "%ld\t%s", bytes,path);
    writeRegister(&reg);
}

const char *actionString(int action){
    switch (action){
        case CREATE:
            return "CREATE";
        case EXIT:
            return "EXIT";
        case RECV_SIGNAL:
            return "RECV_SIGNAL";
        case SEND_SIGNAL:
            return "SEND_SIGNAL";
        case RECV_PIPE:
            return "RECV_PIPE";
        case SEND_PIPE:
            return "SEND_PIPE";
        case ENTRY:
            return "ENTRY";
        default:
            return "";
    }
}

const char *signalString(int signal){
    switch(signal){
        case SIGINT:
            return "SIGINT";
        case SIGSTOP:
            return "SIGSTOP";
        case SIGCONT:
            return "SIGCONT";
        case SIGTERM:
            return "SIGTERM";
        default:
            return "";
    }
}