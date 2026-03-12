#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>
#include <linux/kd.h>
#include <termios.h>

// TODO: Set boot partition for recovery
// #define BOOT_PT 

void halt()
{
    while (1)
        sleep(1);
}

void init_console()
{
    int fd;

    mount("devtmpfs", "/dev", "devtmpfs", MS_NOSUID, NULL);

    fd = open("/dev/console", O_RDWR | O_NOCTTY);
    if (fd >= 0) {
        dup2(fd, 0);
        dup2(fd, 1);
        dup2(fd, 2);
        if (fd > 2)
            close(fd);
    }
}

void mount_filesystems()
{
    mount("proc",     "/proc",    "proc",     0, NULL);
    mount("sysfs",    "/sys",     "sysfs",    0, NULL);
    mkdir("/dev/pts", 0755);
    mount("devpts",   "/dev/pts", "devpts",   0, "gid=5,mode=620");
    mkdir("/dev/shm",  0755);
    mount("tmpfs",    "/dev/shm", "tmpfs",    0, NULL);
}

void mount_boot()
{
    if (mount(BOOT_PT, "/mnt", "ext4", MS_MGC_VAL, NULL) == 0) {
        puts("[+] Good! your boot partition is successfully mount!");
    } else {
        printf("[!] Mount failed: %s\n", strerror(errno));
        puts("[!] Keep in mind that you need to mount boot partition manually.");
    }
}

void ensure_tty1()
{
    if (access("/dev/tty1", F_OK) != 0) {
        mknod("/dev/tty1", S_IFCHR | 0620, makedev(4, 1));
        chown("/dev/tty1", 0, 5);
    }
}

void setup_terminal()
{
    int fd;

    setsid();

    close(0);
    close(1);
    close(2);

    fd = open("/dev/tty1", O_RDWR);
    if (fd < 0)
        fd = open("/dev/console", O_RDWR);
    if (fd < 0)
        _exit(127);

    ioctl(fd, TIOCSCTTY, 0);

    if (fd != 0)
        dup2(fd, 0);
    dup2(0, 1);
    dup2(0, 2);
    if (fd > 2)
        close(fd);

    tcsetpgrp(0, getpid());

    setenv("TERM",  "linux", 1);
    setenv("HOME",  "/root", 1);
    setenv("SHELL", "/bin/bash", 1);
    setenv("PATH",  "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin", 1);
    setenv("USER",  "root", 1);

    chdir("/root");
}

void spawn_shell()
{
    pid_t pid;
    char *const argv[] = { "/bin/bash", "--login", NULL };

    pid = fork();

    if (pid == 0) {
        setup_terminal();
        execvp(argv[0], argv);
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

void reap_zombies(int sig)
{
    (void) sig;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}
