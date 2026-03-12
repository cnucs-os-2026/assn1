#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void init_console();
void mount_filesystems();
void ensure_tty1();
void mount_boot();
void spawn_shell();
void reap_zombies(int);

int main(void)
{
    signal(SIGCHLD, reap_zombies);
    signal(SIGINT,  SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    init_console();
    mount_filesystems();
    ensure_tty1();

    mount_boot();

    puts("[!] Hello My Linux!");

    while (1) {
        spawn_shell();
        sleep(1);
    }

    return 0;
}
