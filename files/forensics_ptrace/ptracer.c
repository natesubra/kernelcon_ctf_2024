#define _GNU_SOURCE
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h> // Include syscall numbers
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#ifndef SYS_sysinfo // If SYS_sysinfo is not defined, define it manually
#define SYS_sysinfo 99 // Example: 99 is the syscall number for sysinfo on x86_64
#endif

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program> [args...]\n", argv[0]);
        exit(1);
    }

    pid_t child_pid = fork();

    if (child_pid == 0) {
        // Child process
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        // Execute the target binary with arguments
        execvp(argv[1], argv + 1);
        // If execvp returns, there was an error
        perror("execvp");
        exit(1);
    } else {
        // Parent process
        int status;
        struct user_regs_struct regs;

        waitpid(child_pid, &status, 0); // Wait for child to stop

        while (1) {
            // Advance to next syscall entry/exit
            ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL);
            waitpid(child_pid, &status, 0);

            if (WIFEXITED(status)) break; // Exit loop if child has exited

            // Get registers and check for sysinfo syscall
            ptrace(PTRACE_GETREGS, child_pid, NULL, &regs);

#ifdef __x86_64__
            if (regs.orig_rax == SYS_sysinfo) {
                // This is a sysinfo syscall, handle accordingly
                printf("sysinfo syscall intercepted\n");
                // Modify regs if needed

                // Inside loop, after obtaining register values
                printf("Register values (as unsigned long):\n");
                printf("RAX: %lu\n", (unsigned long)regs.rax); // System call number (result after syscall)
                printf("RBX: %lu\n", (unsigned long)regs.rbx);
                printf("RCX: %lu\n", (unsigned long)regs.rcx); // 4th argument to the system call, and new instruction pointer (RIP) after syscall
                printf("RDX: %lu\n", (unsigned long)regs.rdx); // 3rd argument to the system call
                printf("RSI: %lu\n", (unsigned long)regs.rsi); // 2nd argument to the system call
                printf("RDI: %lu\n", (unsigned long)regs.rdi); // 1st argument to the system call
                printf("RBP: %lu\n", (unsigned long)regs.rbp);
                printf("RSP: %lu\n", (unsigned long)regs.rsp); // Stack pointer
                printf("RIP: %lu\n", (unsigned long)regs.rip); // Instruction pointer
                printf("RFLAGS: %lu\n", (unsigned long)regs.eflags); // CPU flags
                printf("ORIG_RAX: %lu\n", (unsigned long)regs.orig_rax); // Original syscall number

                // Inside loop, after obtaining register values
                printf("Pointer values (as unsigned long):\n");
                printf("RCX: %lu (Points to: %lx)\n", (unsigned long)regs.rcx, ptrace(PTRACE_PEEKDATA, child_pid, regs.rcx, NULL));
                printf("RDX: %lu (Points to: %lx)\n", (unsigned long)regs.rdx, ptrace(PTRACE_PEEKDATA, child_pid, regs.rdx, NULL));
                printf("RSI: %lu (Points to: %lx)\n", (unsigned long)regs.rsi, ptrace(PTRACE_PEEKDATA, child_pid, regs.rsi, NULL));
                printf("RDI: %lu (Points to: %lx)\n", (unsigned long)regs.rdi, ptrace(PTRACE_PEEKDATA, child_pid, regs.rdi, NULL));

            }
#endif
            // Let the syscall proceed
            ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL);
        }
    }
    return 0;
}
