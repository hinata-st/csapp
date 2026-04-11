/*
 * mysystem.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

int mysystem(const char *command)
{
    if (command == NULL)
        return -1;

#ifdef _WIN32
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    DWORD exit_code = 0;
    char cmdline[1024];

    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);

    snprintf(cmdline, sizeof(cmdline), "cmd.exe /C %s", command);

    if (!CreateProcessA(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        fprintf(stderr, "CreateProcess failed: %lu\n", GetLastError());
        return -1;
    }

    printf("child pid: %lu\n", (unsigned long)pi.dwProcessId);

    WaitForSingleObject(pi.hProcess, INFINITE);
    if (!GetExitCodeProcess(pi.hProcess, &exit_code))
    {
        fprintf(stderr, "GetExitCodeProcess failed: %lu\n", GetLastError());
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return -1;
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return (int)exit_code;
#else
    pid_t pid;
    int status;

    pid = fork();
    if (pid == -1)
    {
        fprintf(stderr, "fork error: %s\n", strerror(errno));
        return -1;
    }

    if (pid == 0)
    {
        execl("/bin/sh", "sh", "-c", command, (char *)NULL);
        _exit(127);
    }

    printf("child pid: %d\n", pid);

    if (waitpid(pid, &status, 0) > 0)
    {
        if (WIFEXITED(status))
            return WEXITSTATUS(status);
        if (WIFSIGNALED(status))
            return WTERMSIG(status);
    }

    return -1;
#endif
}

int main(int argc, char *argv[])
{
    int code;
#ifdef _WIN32
    code = mysystem("exit-code.exe");
#else
    code = mysystem("./exit-code");
#endif
    printf("normally exit, code: %d\n", code);
    fflush(stdout);

#ifdef _WIN32
    code = mysystem("wait-sig.exe");
#else
    code = mysystem("./wait-sig");
#endif
    printf("exit caused by signal, code: %d\n", code);
    fflush(stdout);
    return 0;
}