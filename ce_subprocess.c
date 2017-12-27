#include "ce_subprocess.h"
#include <unistd.h>

// NOTE: stderr is redirected to stdout
static pid_t bidirectional_popen(const char* cmd, int* in_fd, int* out_fd){
     int input_fds[2];
     int output_fds[2];

     if(pipe(input_fds) != 0) return 0;
     if(pipe(output_fds) != 0) return 0;

     pid_t pid = fork();
     if(pid < 0) return 0;

     if(pid == 0){
          close(input_fds[1]);
          close(output_fds[0]);

          dup2(input_fds[0], STDIN_FILENO);
          dup2(output_fds[1], STDOUT_FILENO);
          dup2(output_fds[1], STDERR_FILENO);

          // TODO: run user's SHELL ?
          execl("/bin/sh", "/bin/sh", "-c", cmd, NULL);
     }else{
         close(input_fds[0]);
         close(output_fds[1]);

         *in_fd = input_fds[1];
         *out_fd = output_fds[0];
     }

     return pid;
}

bool ce_subprocess_open(CeSubprocess_t* subprocess, const char* command){
     subprocess->pid = bidirectional_popen(command, &subprocess->stdin_fd, &subprocess->stdout_fd);
     if(subprocess->pid == 0) return false;
     subprocess->stdin = fdopen(subprocess->stdin_fd, "w");
     subprocess->stdout = fdopen(subprocess->stdout_fd, "r");
     return true;
}

void ce_subprocess_close_stdin(CeSubprocess_t* subprocess){
     if(!subprocess->stdin) return;

     fclose(subprocess->stdin);
     subprocess->stdin = NULL;
     subprocess->stdin_fd = 0;
}

void ce_subprocess_close(CeSubprocess_t* subprocess){
     ce_subprocess_close_stdin(subprocess);

     fclose(subprocess->stdout);
     // wait for the subprocess to complete. this should always be successful
     // unless we are interrupted by a signal handler... I think
     pid_t w;
     int status;
     do{
          w = waitpid(subprocess->pid, &status, 0);
     }while(w == -1);
}
