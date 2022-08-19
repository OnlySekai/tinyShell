#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>

typedef sighandler_t handler_t;
void Kill(pid_t pid, int sig);
void Sigfillset(sigset_t *mask);
void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
handler_t Signal(int signum, handler_t handler);
char* Strdup(const char *);
void	SysErr(const char *s);
