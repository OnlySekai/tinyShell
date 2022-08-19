#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

typedef struct 
{
	const char *name;
	int (*func)(int, const char **);
} builtin_table_t;

int builtin_jobs (int argc, const char **argv);
int builtin_fg (int argc, const char **argv);
int builtin_bg (int argc, const char **argv);
int builtin_terminate (int argc, const char **argv);
int builtin_stop (int argc, const char **argv);
int builtin_setenv (int argc, const char **argv);
int builtin_getenv (int argc, const char **argv);
int builtin_help (int argc, const char **argv);
int builtin_source (int argc, const char **argv);
int builtin_exit (int argc, const char **argv);
int builtin_listenv (int argc, const char **argv);
int builtin_cd (int argc, const char **argv);
int builtin_path (int argc, const char **argv);
int builtin_addpath (int argc, const char **argv);

extern builtin_table_t table[];

