# intro signal

## signal -> man 7 signal

signal handler -> signal

```c
handler_t signal_INT_HANDLER()
{

}
```

self-custom function

```c
fork()
```

A child created via fork(2) inherits a copy of its parent's signal disposition. During an execve(2)

### sending signal 

kill

```c
kill(pid, SIGINT); //this process
kill(-pid, SIGINT); // process group
```

### wait for a signal to be caught

```c
pause() 

sigsuspend()
```

data race

### signal mask and pending signals

```c
block()
.. 
.. 
..
unblock()
```

## waitpid man waitpid

defination: wait for process to change state

- terminated
- stopped
- continued

```c
pid_t waitpid(pid_t pid, int *status, int options);
```

the state change can including

- the child is terminated exit(0)
- the child is stopped by a signal
- the child is resume by a signal

In the case of a terminated child, performing a wait allows the system to release the resources

if a wait is not performed, then the terminated child remain in a "zombie" state(see NOTES below).

**pid**

- <-1 -255
- -1 fork()

the value options is an OR of zero or more of the following constants

- WNOHANG
    - release block
- WUNTRACED
    - child process -> ST
- WCONTINUED
    - resume


**wstatus**

```c
if (WIFEXITED(wstatus)) // exit(0), exit(1)
    return WEXITSTATUS(wstatus);//this macro should be employed only if WIFEXITED returned ture
```

```c
if (WIFSIGNALED(wstatus)) // terminated by signal
    return WTERMSIG(wstatus); // sigkill, sigint
```

```c
if (WIFSTOPPED(wstatus)) // stopped by signal
    return WSTOPSIG(wstatus); // sigstop, sigtstp
```

```c
if (WIFCONTINUED(wstatus)) // resumed by signal
    return 0;
```

## kill

define -> send a signal to a precess

```c
kill(pid, SIGCONT)
```

Particularly useful 
signals include HUP, INT, STOP, CONT, and 0

## sigprocmask, man sigprocmask

signal blocking unblocking, setmask

```c
process -> set of blocked signals
/* prototype for the glibc wrapper function */
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
```

- SIG_BLOCK
- SIG_UNBLOCK
- SIG_SETMASK

```c
sigset_t mask_all; prev_all;

sigemptyset(&mask_all);
sigfillset(&mask_all);

// block all the signal
// prev_all stores the priovus blocked set
sigprocmask(SIG_BLOCK, &mask_all, &prev_all);

sigprocmask(SIG_UNBLOCK, &prev_all, NULL);
```

# shllab

```c
// signit_handler - The kernal sends a SIGNIT to the shell whenever the user types ctrl-c at the keyboard. catch it and send it along to the foreground job
void signit_handler(int sig)
{
    pid_t pid = fgpid(jobs);
    if (pid == 0)
        return;
    kill(-pid, sig);
}
```

```c
// sigstp_handler - The kernel sends a SIGSTP to the shell whenever the user types ctrl-z at the keyboard. catch it and suspend the foregtound job by sending it a SIGSTP
void sigstp_handler(int sig)
{
    pid_t pid = fgpid(jobs);
    if (pid == 0)
        return;
    kill(-pid, sig);
}
```

```c
// sigchld_handler - The kernal sends a SIGCHILD to the shell whenever a child job terminates (becomes a zombie), or stops because it received s SIGSTOP or SIGSTP. The handler reaps a;; available zombie children, but doesn't wait for any other currently runnig children to terminate.
void sigchld_handler(int sig) 
{
    int olderrno = errno;
    pid_t pid;
    int status;

    while((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0)
    {
        // enter here means that one of child has changed status
        if (WIFEXITED(status))
        {
            deletejob(jobs, pid);
            // exit, normally, exit(0), exit(1)
        }
        else if(WIFSIGNALED(status))
        {
            int jid = pid2jid(pid);
            printf("Job [%d] (%d) terminated by signal %d\n", jid, pid, WTERMSIG(status));
            deletejob(jobs, pid);
        }
        else if(WIFSTOPPED(status))
        {
            // stop
            struct job_t *job = getjobpid(jobs, pid);
            job->state = ST;
            int jid = pid2jid(pid);
            printf("Job [%d] (%d) stopped by signal %d\n", jid, pid, WSTOPSIG(status));
        }
    }

    errno = olderrno;
    return;
}
```

```c
// waitfg - block until process pid is no longer the foreground process
void waitfg(pid_t pid)
{
    struct job_t *job = getjobpid(jobs, pid);
    if (job == NULL)
        return;

    while(fgpid(jobs) == pid){// spin
    }
}
```

```c
// builtin_cmd - If the user has typed a built-in command then execute it immediately.
int builtin_cmd(char **argv) 
{
    if (strcmp(argv[0], "quit") == 0) {
        exit(0);
    }
    else if (strcmp(argv[0], "jobs") == 0) {
        listjobs(jobs);
        return 1;
    }
    else if (strcmp(argv[0], "bg") == 0) {
        do_bgfg(argv);
        return 1;
    }
    else if (strcmp(argv[0], "fg") == 0) {
        do_bgfg(argv);
        return 1;
    }
    else if (strcmp(argv[0], "&") == 0) {
        return 1;
    }
    return 0;     /* not a builtin command */
}
```

```c
/* 
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv) 
{
    struct job_t *job = NULL;
    char *tmp;
    int jid;
    pid_t pid;

    tmp = argv[1];

    // if id does not exist
    if (tmp == NULL){
        printf("%s command requires PID or %%jobid argument\n", argv[0]);
        return; 
    }

    // if it is a jid
    if (tmp[0] == '%'){
        jid = atoi(&tmp[1]);
        // get job
        job = getjobjid(jobs, jid);
        if (job == NULL){
            printf("%s: No such job\n", tmp);
            return;
        }
        else{
            // get the pid if a valid job for later to kill
            pid = job->pid;
        }
    }

    // if it is a pid
    else if (isdigit(tmp[0])){
        // get pid
        pid = atoi(tmp);
        // get job
        job = getjobpid(jobs, pid);
        if (job == NULL){
            printf("(%d): No such process\n", pid);
            return;
        }
    }
    else{
        printf("%s: argument must be a PID or %%jobid\n", argv[0]);
        return;
    }

    // bg %1
    // kill for each time
    kill(-pid, SIGCONT);

    if (strcmp(argv[0], "fg") == 0){
        job->state = FG;
        waitfg(pid);
    }
    else{
        job->state = BG;
        printf("[%d] (%d) %s", job->jid, job->pid, job->cmdline);
    }
}
```

```c
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline) 
{
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    sigset_t mask;        /* Signal mask for critical sections */

    strcpy(buf, cmdline);
    // parase line
    bg = parseline(buf, argv);
    if(!builtin_cmd(argv)){
        // block first
        sigemptyset(&mask);
        sigaddset(&mask, SIGCHLD);
        sigprocmask(SIG_BLOCK, &mask, NULL);
        // forking
        if ((pid = fork()) < 0){
            unix_error("forking error");
        }

        // child
        else if (pid == 0){
            sigprocmask(SIG_UNBLOCK, &mask, NULL); // unblock SIGCHLD
            setpgid(0, 0);                          // set the process to be independent
            // check if command is there
            if (execve(argv[0], argv, environ) < 0){
                printf("%s: Command not found.\n", argv[0]);
                exit(1);
            }
        }

        else {
            if (!bg){
                addjob(jobs, pid, FG, cmdline);
            }
            else {
                addjob(jobs, pid, BG, cmdline);
            }
            sigprocmask(SIG_UNBLOCK, &mask, NULL); // unblock SIGCHLD

            if (!bg){
                waitfg(pid);
            }
            else {
                // print job info
                printf("[%d] (%d) %s", pid2jid(pid), pid, cmdline);
            }
        }
    }
    


}
```








