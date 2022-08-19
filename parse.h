#pragma once
#include<stdio.h>
#include"job.h"

typedef struct 
{
	char *cmd;
	int argc;
	char **argv;
	int fg;
	char err[100];
} parse_t;
 
void parse_free(parse_t *p);
parse_t *parse_alloc();
parse_t *parse_alloc_readline(FILE *f);
void parse_dump(parse_t *p);
int parse_str(parse_t *p);
void eval(parse_t *p);
