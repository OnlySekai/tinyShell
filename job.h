#pragma once
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<unistd.h>
#include<signal.h>
#include<string.h>
#include<errno.h>
#include<sys/wait.h>  
#include<stdbool.h>
#include"wrap.h"
#define MAX_JOB 32

typedef int (*builtin_prog_t)(int, const char **);
typedef enum job_state_enum
{
	EMPTY=0,
	CREATED,
	FG,
	BG,
	ST
} job_state_t;

typedef struct job_struct
{
	pid_t					pid;
	job_state_t			state;
	int					status;
	char					*cmd;
	int					argc;
	char					**argv;
	char					notified;
} job_t;
extern job_t job_array[MAX_JOB];

//Interface
void	job_int(int id);
void	job_term(int id);
void	job_list();
int	job_create(int, char**, char*);
void	job_init();
void	job_launch(int id, int fg);
void	job_stop(int id);
void	job_put_fg(int id, int cont);
void	job_put_bg(int id, int cont);
int	job_find_fg();
bool	job_is_fg(int id);
bool	job_is_valid(int id);
int	job_find_pid(pid_t pid);
job_t	*job_pointer(int id);
int	job_id_of(job_t *j);
void	job_force_quit();
void	job_info_printf(int id, const char *, ...);
void	job_update();

//Use if know
void	job_wait(int id);
void	job_cont(int id);
void	job_free_id(int id);
