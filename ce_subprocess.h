#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <sys/wait.h>

typedef struct{
     pid_t pid;
     // you should either use the file descriptor, or the FILE pointer.
     // do not mix and match
     int stdin_fd;
     int stdout_fd;
     FILE *stdin;
     FILE *stdout;
}CeSubprocess_t;

// run the provided shell command as a subprocess
bool ce_subprocess_open(CeSubprocess_t* subprocess, const char* command);
// signal to the subprocess that you are done providing it input by closing stdin
void ce_subprocess_close_stdin(CeSubprocess_t* subprocess);
// close all subprocess fds and fps and wait for the subprocess to complete
void ce_subprocess_close(CeSubprocess_t* subprocess);
