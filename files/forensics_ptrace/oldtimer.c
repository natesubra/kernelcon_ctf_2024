#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <string.h>
#include <sys/sysinfo.h>

void modify_uptime(pid_t child_pid, unsigned long addr) {
    struct sysinfo info;

    // Read the sysinfo struct from child's memory
    errno = 0;
    long data;
    for (unsigned long i = 0; i < sizeof(info); i += sizeof(data)) {
        data = ptrace(PTRACE_PEEKDATA, child_pid, addr + i, NULL);
        if (data == -1 && errno != 0) {
            perror("PTRACE_PEEKDATA failed");
            return;
        }
        memcpy((char *)&info + i, &data, sizeof(data));
    }

    printf("Original uptime: %lu\n", info.uptime);

    // Modify the uptime value
    info.uptime = 999999999; // Uptimer

    // Write the modified sysinfo struct back to child's memory
    for (unsigned long i = 0; i < sizeof(info); i += sizeof(data)) {
        memcpy(&data, (char *)&info + i, sizeof(data));
        if (ptrace(PTRACE_POKEDATA, child_pid, addr + i, data) == -1) {
            perror("PTRACE_POKEDATA failed");
            return;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program> [args...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t child_pid = fork();

    if (child_pid == 0) {
        // Child process
        ptrace(PTRACE_TRACEME, 0, NULL, NULL); // Allow the parent to trace
        raise(SIGSTOP); // Stop until the parent is ready to trace
        execvp(argv[1], argv + 1);
        perror("execvp"); // execvp only returns on error
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        waitpid(child_pid, &status, WUNTRACED); // Wait for the child to stop

        struct user_regs_struct regs;
        while (1) {
            ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL);
            waitpid(child_pid, &status, 0);

            if (WIFEXITED(status)) break; // Exit loop if child has exited

            ptrace(PTRACE_GETREGS, child_pid, NULL, &regs);
            if (regs.orig_rax == SYS_sysinfo) {
                // Check if this is the entry or exit of the syscall
                if (regs.rax == 0) { // Assuming syscall successful, modify on exit
                    modify_uptime(child_pid, regs.rdi); // rdi holds the pointer to the sysinfo struct
                }
            }
        }
    }

    return 0;
}