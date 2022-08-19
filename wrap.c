#include"wrap.h"
void SysErr(const char *s)
{
	perror(s);
	exit(-1);
}
  
void Kill(pid_t pid, int sig)
{
    int ret;
    if((ret = kill(pid, sig)) == -1) {
        SysErr("kill() failed");
    }
}
void Sigfillset(sigset_t *mask)
{
    if(sigfillset(mask) < 0)
        SysErr("sigfillset() failed");
    return;
}
void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
    if(sigprocmask(how, set, oldset) < 0) {
        SysErr("sigprocmask() failed");
    }
    return;
}
handler_t Signal(int signum, handler_t handler)
{
    struct sigaction action, oldaction;
    
    action.sa_handler = handler;
    Sigfillset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    
    if(sigaction(signum, &action, &oldaction) < 0)
        SysErr("sigaction() failed");
    return (oldaction.sa_handler);

}

char* Strdup(const char *s)
{
	char *ret=strdup(s);
	if(!ret)SysErr("strdup() failed");
	return ret;
}
