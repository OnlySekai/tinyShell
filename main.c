#include"parse.h"
 
void promt()
{
	job_update();
	printf("[timyshell %s]> ", basename(get_current_dir_name()));   
}

void sig_han_quit(int signum)
{
	job_force_quit();
	exit(0);
}

void sig_han_int(int signum)
{
	int id=job_find_fg();
	if(id>=0) job_int(id);
}

void sig_han_tstp(int signum)
{
	int id=job_find_fg();
	if(id>=0) job_stop(id);
}

int main()
{
	Signal(SIGINT, sig_han_int);    //^C
	Signal(SIGTSTP, sig_han_tstp);  //^Z
	Signal(SIGQUIT, sig_han_quit);  //^D
	job_init();
	while(1)
	{
		promt();
		parse_t *ps=parse_alloc_readline(stdin);
		if(!ps)break;
      eval(ps);
		parse_free(ps);
	}
	job_force_quit();
	exit(0);
}
