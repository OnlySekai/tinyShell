#include"builtin.h"
#include"parse.h"
#include"job.h"
 
builtin_table_t table[] = {
	{"exit", builtin_exit},
	{"jobs", builtin_jobs},
	{"fg",	builtin_fg},
	{"bg",	builtin_bg},
	{"terminate", builtin_terminate},
	{"stop",	builtin_stop},
	{"setenv",	builtin_setenv},
	{"getenv",	builtin_getenv},
	{"help",		builtin_help},
	{"source",	builtin_source},
	{"listenv", builtin_listenv},
	{"cd", builtin_cd},
	{"path", builtin_path},
	{"addpath", builtin_addpath},
	{NULL, NULL}
};

int builtin_addpath(int argc, const char **argv)
{
	if(argc<2)
	{
		puts("Usage addpath <path1> <path2> : Add paths to PATH environment");
		return 0;
	}
	const char *old=getenv("PATH");
	if(!old)
	{
		perror("getenv():");
		return 0;
	}
	size_t len=strlen(old);
	for(int i=1;i<argc;++i)
		len+=strlen(argv[i])+1;
   char *new=malloc(len+1);
	char *top=new;
	for(const char *c=old; *c;++c)
		*(top++)=*c;
	for(int i=1;i<argc;++i)
	{
		*(top++)=':';
		for(const char *c=argv[i]; *c;++c)
			*(top++)=*c;
	}
	setenv("PATH", new, 1);
	free(new);
	return 0;
}
                                
int builtin_path(int argc, const char **argv)
{
	if(argc<0)
	{
		puts("Usage path: echo all path");
		return 0;
	}
	const char *env_path=getenv("PATH");
	if(!env_path)
	{
		perror("getenv(): ");
		return 0;
	}
	char *s = Strdup(env_path);
	for(char *c=s; *c;++c)
	{
		if(*c==':')putchar('\n');
		else putchar(*c);
	}
	free(s);
	putchar('\n');
	return 0;
}

int builtin_cd(int argc, const char **argv)
{
	if(argc<0)
	{
		puts("Usage cd : change cwd");
		return 0;
	}
	int ret;
	if(argc==1)
		ret=chdir(getenv("HOME"));
	else ret=chdir(argv[1]);
	if(ret<0)perror("");
	return 0;
}

int builtin_jobs(int argc, const char **argv)
{
	if(argc<0)
	{
		puts("Usage jobs: print job list");
		return 0;
	}
	job_list();
	return 0;
}
int builtin_exit(int argc, const char **argv)
{
	if(argc<0)
	{
		puts("Usage exit: try to kill all job and exit");
		return 0;
	}
	job_force_quit();
	exit(0);
	return 0;
}  
int builtin_fg(int argc, const char **argv)
{
	if(argc!=2)
	{
		puts("Usage fg <job id> : put 1 job in foreground");
		return 0;
	}
	int id=strtol(argv[1], NULL, 10);
	job_put_fg(id, 1);
	return 0;
}  
int builtin_bg(int argc, char const **argv)
{
	if(argc<2)
	{
		puts("Usage bg <job id 1> <job id 2> ... : put jobs in background");
		return 0;
	}
	for(int i=1;i<argc;++i)
	{
		int id=strtol(argv[i], NULL, 10);
		job_put_bg(id, 1);
	}
	return 0;
}  
int builtin_terminate(int argc, const char **argv)
{
	if(argc<2)
	{
		puts("Usage terminate <job id 1> <job id 2> ... : terminate jobs");
		return 0;
	}
	for(int i=1;i<argc;++i)
	{
		int id=strtol(argv[i], NULL, 10);
		job_term(id);
	}
	return 0;         
}  
int builtin_stop(int argc, const char **argv)
{
	if(argc<2)
	{
		puts("Usage stop <job id 1> <job id 2> ... : stop jobs");
		return 0;
	}
	for(int i=1;i<argc;++i)
	{
		int id=strtol(argv[i], NULL, 10);
		job_stop(id);
	}
	return 0;
}  
int builtin_setenv(int argc, const char **argv)
{
	if(argc!=4)
	{
		puts("Usage setenv <env> <val> <overwrite> : set env to val, if exist overwrite or not");
		return 0;
	}
   int o=0;
	if(!strcmp(argv[3], "1"))o=1;
	if(!strcmp(argv[3], "true"))o=1;
   if(setenv(argv[1], argv[2], o)<0)
		perror("error: ");
	return 0;
}  
int builtin_getenv(int argc, const char **argv)
{
	if(argc<2)
	{
		puts("Usage getenv <env1> <env2> ... : get env list");
		return 0;
	}
	for(int i=1;i<argc;++i)
	{
		const char *val=getenv(argv[i]);
		if(!val)printf("env %s not found\n", argv[i]);
		puts(val);
	}
	return 0;
}  
int builtin_help(int argc, const char **argv)
{
	puts("builtin command list:");
	for(int i=0; table[i].name; ++i)
	{
		if(!strcmp("help", table[i].name))continue;
		putchar('\t');
		table[i].func(-1, NULL);
	}
	return 0;
}   
int builtin_source(int argc, const char **argv)
{
	if(argc<2||argc>3)
	{
		puts("Usage source <file.tsh> <echo> : execute script file, echo or not (default false)");
		return 0;
	}
	const char *name=argv[1];
   const char *filename=basename(name);
   int echo=0;
	if(argc==3)
	{
		if(!strcmp(argv[2], "true"))
			echo=1;
		if(!strcmp(argv[2], "1"))
			echo=1;
	}  
	for(int i=strlen(filename)-1;i>=0;--i)
	{
		if(filename[i]=='.')
		{
			if(i==0)break;
			if(strcmp(filename+i+1, "tsh"))break;
			goto passed;
		}
	}
	puts("Invalid file name");
	return 0;

	passed:
	FILE *f=fopen(argv[1], "r");
	if(!f)
	{
		perror("source:");
		return 0;
	}

	while(1)
	{
		
		parse_t *ps=parse_alloc_readline(f);
		if(!ps) return 0;
		if(echo)
		{
			printf("[%s %s]> ", filename, basename(get_current_dir_name()));   
			printf("%s", ps->cmd);
			fflush(stdout);
		}
		eval(ps);
		parse_free(ps);
	}
	return 0;
}

int builtin_listenv(int argc, const char **argv)
{
	if(argc<0)
	{
		puts("Usage listenv : list all enviroment variable");
		return 0;
	}

	for(char **i=environ;*i;++i)
		puts(*i);
	return 0;
}
