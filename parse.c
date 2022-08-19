#include"parse.h"
#include"builtin.h"

void parse_free(parse_t *p) {
	free(p->cmd);
	for(int i=0;i<p->argc;++i)
		free(p->argv[i]);
	free(p->argv);
	free(p);
}

parse_t *parse_alloc()
{
	parse_t *p=malloc(sizeof(parse_t));
	if(!p) SysErr("malloc() failed");
	memset(p, 0, sizeof(parse_t));
	return p;
}

parse_t *parse_alloc_readline(FILE *file)
{
	parse_t *p=parse_alloc();
	size_t cnt=0;
	if(getline(&(p->cmd), &cnt, file)<0)
	{
		if(feof(file))
		{
			job_force_quit();
			return 0;
		}
		perror("getline() failed:");
		free(p->cmd);
		job_force_quit();
		exit(0);
		return 0;
	}
	return p;
}

void parse_dump(parse_t *p)
{
	printf("cmd: %s\n fg: %i\n argc: %i\n", p->cmd, p->fg, p->argc);
	for(int i=0;p->argv[i];++i)
		printf("\t argv %i: %s\n", i, p->argv[i]);
}

int parse_str(parse_t *p)
{
	char *cmd=p->cmd;
	int i=0;
	//\n -> space
	while(cmd[i])
	{
		if(cmd[i]=='\n')cmd[i]=' ';
		++i;
	}

	//check syntax + count arg
	int count_arg = 0;
	i=0;
	while(1)
	{
		while(cmd[i]==' ') ++i;
		if(cmd[i]==0) break;
		if(cmd[i]=='&')
		{
			int j=i;
			do ++j;
			while(cmd[j]==' ');
			if(cmd[j]!=0)
			{
				strcpy(p->err, "expected only one '&' at end of cmd");
				return -1;
			}
			break;
		}
    //character found
		++count_arg;
		//skip character
		while(cmd[i]!=' '&&cmd[i]!='&'&&cmd[i]) ++i;
	}
	if(!count_arg)
	{
		strcpy(p->err, "");
		return -1;
	}

	//syntax check pass
	i=0;
	p->argc=count_arg;
	p->argv=malloc(sizeof(void*)*(count_arg+1));
	if(!p->argv)SysErr("malloc() failed");
	count_arg=0;
	char *token;
 
	while(1)
	{
		while(cmd[i]==' ') ++i;
		if(cmd[i]==0) 
		{
			p->fg=1;
			break;
		}
		if(cmd[i]=='&')
		{
			p->fg=0;
			break;
		}
    //character found
    token=cmd+i;

		//skip character
		while(cmd[i]!=' '&&cmd[i]!='&'&&cmd[i]) ++i;
		int k=cmd[i];
		cmd[i]=0;
		p->argv[count_arg] = Strdup(token);
		++count_arg;
		cmd[i]=k;
	}
	p->argv[count_arg]=NULL;

	return 0;
}
void eval(parse_t *ps)
{
	if(parse_str(ps)<0)
	{
		if(ps->err[0])
			fprintf(stderr, "%s\n", ps->err);
	}
	else
	{
		for(int i=0;table[i].name;++i)
			if(!strcmp(table[i].name, ps->argv[0]))
			{
				table[i].func(ps->argc, (const char **)ps->argv);
				goto done;
			}
		if(!strcmp("exit", ps->argv[0]))
		{
			job_force_quit();
			exit(0);
		}
		else if(!strcmp("jobs", ps->argv[0]))
			job_list();   
		else if(!strcmp("fg", ps->argv[0]))
		{
			if(!ps->argv[1])printf("Usage fg <job id>\n");
			else
			{
				int id=strtol(ps->argv[1], NULL, 10);
				job_put_fg(id, 1);
			}
		}
		else if(!strcmp("bg", ps->argv[0]))
		{
			if(!ps->argv[1])
				printf("Usage bg <job id>\n");
			else
			{
				int id=strtol(ps->argv[1], NULL, 10);
				job_put_bg(id, 1);
			}
		}
		else
		{
			int id=job_create(ps->argc, ps->argv, ps->cmd);
			if(id>=0) job_launch(id, ps->fg);
			else printf("too many jobs!\n");
		}
		done:
	} 
}
