#include<stdarg.h>
#include"job.h"

job_t job_array[MAX_JOB];

void job_launch(int id, int fg)
{
	job_t *j=job_array+id;
	j->pid=fork();
	if(j->pid<0)
		SysErr("fork() failed");
	else if(j->pid==0)
	{
		if(setpgid(0, 0)<0)
			SysErr("setpgid() failed");
		Signal(SIGINT,	SIG_DFL);
		Signal(SIGTSTP, SIG_DFL);
		Signal(SIGTERM, SIG_DFL);
		Signal(SIGQUIT, SIG_DFL);
		Signal(SIGCHLD, SIG_DFL);

		execvp(j->argv[0], j->argv);

		//If control reach this, then there error in execv
		puts(strerror(errno));
		exit(0);  
	}
	
	//father
	if(fg) job_put_fg(id, 0);
	else job_put_bg(id, 0);
	return;
}

void job_stop(int id)
{
	if(id>=MAX_JOB||id<0)
	{
		printf("Invalid id\n");
		return;
	}
	job_t *j=job_array+id;
	if(j->state==EMPTY)
	{
	  	printf("Invalid id\n");
		return;
	}
	
	j->state=ST;
	if(kill(j->pid, SIGSTOP)<0)
		SysErr("kill() failed");
}

void job_put_fg(int id, int cont)
{
	if(id>=MAX_JOB||id<0)
	{
		printf("Invalid id\n");
		return;
	}
	job_t *j=job_array+id;
	if(j->state==EMPTY)
	{
	  	printf("Invalid id\n");
		return;
	}
	j->state=FG;
	if(cont)job_cont(id);
	job_wait(id);
}

void job_put_bg(int id, int cont)
{
	if(id>=MAX_JOB||id<0)
	{
		printf("Invalid id\n");
		return;
	}
	job_t *j=job_array+id;
	if(j->state==EMPTY)
	{
	  	printf("Invalid id\n");
		return;
	}
	
	job_info_printf(id, "background");
	j->state=BG;
	if(cont)job_cont(id);
}

int job_find_fg()
{
	for(int i=0;i<MAX_JOB;++i)
		if(job_array[i].state==FG)return i;
	return -1;
}

void job_int(int id)
{
	if(id>=MAX_JOB||id<0)
	{
		printf("Invalid id\n");
		return;
	}
	job_t *j=job_array+id;
	if(j->state==EMPTY)
	{
	  	printf("Invalid id\n");
		return;
	} 
	if(j->state==EMPTY)
	  	printf("Invalid id\n");
	Kill(j->pid, SIGINT);
	if(j->state==ST)
		job_cont(id);
}
void job_term(int id)
{
	if(id>=MAX_JOB||id<0)
	{
		printf("Invalid id\n");
		return;
	}
	job_t *j=job_array+id;
	if(j->state==EMPTY)
	{
	  	printf("Invalid id\n");
		return;
	} 
	if(j->state==EMPTY)
	  	printf("Invalid id\n");
	Kill(j->pid, SIGKILL);
	if(j->state==ST)
		job_cont(id);
}

int job_id_of(job_t *j)
{
	return (int)(j-job_array);
}

bool job_is_fg(int id)
{
	return job_array[id].state==FG;
}

bool job_is_valid(int id)
{
	return job_array[id].state!=EMPTY;
}

int job_find_pid(pid_t pid)
{
	for(int i=0;i<MAX_JOB;++i)
	{
		if(job_array[i].state==EMPTY)continue;
		if(job_array[i].pid==pid)return i;
	}
	return -1;
}

void job_kill_all(int signum)
{
	for(int i=0;i<MAX_JOB;++i)
	{
		if(job_array[i].state==EMPTY)continue;
		Kill(job_array[i].pid, signum);
	}
}

void job_force_quit()
{
	job_update();
	job_kill_all(SIGTERM);
	job_update();
	job_kill_all(SIGQUIT);
	job_update();
	job_kill_all(SIGKILL);
	job_update();
}

void job_list()
{
	for(int id=0;id<MAX_JOB;++id)
	{
		job_t *j=job_array+id;
		if(j->state==EMPTY)continue;
		job_info_printf(id, "%s", (j->state==ST)?"stopped":(j->state==BG)?"background":"");
	}
}

void job_mark_state(int id, int status)
{
	job_t *j=job_array+id;
	if(WIFEXITED(status))
	{
		if(j->state==FG)
		{
			if(WEXITSTATUS(status))
				job_info_printf(id, "exit with code %i", WEXITSTATUS(status));
			job_free_id(id);
		}
		else 
		{
			if(WEXITSTATUS(status))
				job_info_printf(id, "exit with code %i", WEXITSTATUS(status));
			else 
				job_info_printf(id, "done");
			job_free_id(id);  
		}
	}
	else if(WIFSTOPPED(status))
	{
		j->state=ST;
		job_info_printf(id, "stopped");
	}
	else if(WIFSIGNALED(status))
	{
		job_info_printf(id, "terminated by %s", strsignal(WTERMSIG(status)));
		job_free_id(id);
	}
	else if(WCOREDUMP(status))
	{
		job_info_printf(id, "core dumped");
		job_free_id(id);
	}
	else 
	{ 
		printf("AH read %s at %s pls :>", __FUNCTION__, __FILE__);
	}
}

void job_update()
{
	int status;
	pid_t pid;
	while(1)
	{
		pid=waitpid(WAIT_ANY, &status, WUNTRACED|WNOHANG);
		if(pid==0)break;
		if(pid<0)
		{
			if(errno==ECHILD)
				break;
			perror("waitpid():");
			break;
		}
		job_mark_state(job_find_pid(pid), status);
	}
}

job_t *job_pointer(int id)
{
	return job_array+id;
}

void job_wait(int id)
{
	int status;
	job_t *j=job_array+id;
	if(j->state==EMPTY)return;
	if(waitpid(j->pid, &status, WUNTRACED)<0)
		SysErr("waitpid() failed");
	job_mark_state(id, status);
}

void job_init()
{
	for(int i=0;i<MAX_JOB;++i)
		job_array[i].state=EMPTY;
}

void job_free_id(int id)
{
	job_t *j=job_array+id;
	free(j->cmd);
	for(char **i=j->argv;*i;++i)
		free(*i);
	free(j->argv);
	j->argc=0;
	j->pid=0;
	j->state=EMPTY;
	j->cmd=NULL;
}

void job_cont(int id)
{
	job_t *j=job_array+id;
	if(kill(j->pid, SIGCONT)<0)
		SysErr("kill() failed");
}

int job_create(int argc, char **argv, char *cmd)
{
	int id=0;
	for(;id<MAX_JOB;++id)
		if(job_array[id].state==EMPTY)break;
	if(id==MAX_JOB)return -1;
	job_t *j=job_array+id;
	j->pid=0;
	j->state=CREATED;
	j->cmd=Strdup(cmd);
	j->argc=argc;
	j->argv=malloc(sizeof(void*)*(argc+1));
	j->notified=1;
	for(int i=0;i<argc;++i)
		j->argv[i]=Strdup(argv[i]);
	j->argv[argc]=NULL;
	return id;
}

void job_info_printf(int i, const char *s, ...)
{
	job_t *j=job_array+i;
	printf("[%i] ", i);
	va_list vals;
	va_start(vals, s);
	vfprintf(stdout, s, vals);
	va_end(vals);
	printf("\t\t\t %s\n", j->cmd);
	return;
}

