#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void setup_terminal();
void halt();

static void spawn_shell(void)
{
    pid_t pid;
    char *const argv[] = { "/bin/bash", "--login", NULL };

    // TODO: fork current process

    if (pid == 0) {
        setup_terminal();

	// TODO: replace child with execvp

	perror("execvp");
        _exit(127);
    } else if (pid < 0) {
        printf("[!] fork failed: %s\n", strerror(errno));
        halt();
    }

    int status;
    waitpid(pid, &status, 0);
    printf("[!] Shell exited (status %d). Respawning...\n", status);
}
