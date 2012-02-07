/*********************************************************************************
**   Copyright (C) 2012 Mike Nawrocki <mnawrocki3 at gatech.edu>                **
**                                                                              **
**   Permission to use, copy, modify, and distribute this software for any      **
**   purpose with or without fee is hereby granted, provided that the above     **
**   copyright notice and this permission notice appear in all copies.          **
**                                                                              **
**   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES   **
**   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF           **
**   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR    **
**   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES     **
**   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN      **
**   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF    **
**   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.             **
*********************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define LOGGING 1
#define MAKEARG (1 << 1)
#define TESTARG (1 << 2)

pid_t make();
pid_t test();

static unsigned char optflags;
char *makefile;
char makefilepath[PATH_MAX];
char *testfile;

void parseargs(int argc, char ***argv);

int main(int argc, char **argv)
{
	int ret = 0;
	char *base = getcwd(NULL,0);
	if(argc < 2)
	{
		printf("No directories provided! Aborting!\n");
		exit(-1);
	}
	parseargs(argc, &argv);
	do
	{
		chdir(argv[optind]);
		printf("Making \"%s\"\n", argv[optind]);
		waitpid(make(), &ret, 0);
		if(ret)
		{
			fprintf(stderr, "Make failed! Check \"make.log\" for error output.\n");
			printf("make exit status: %d\n", WEXITSTATUS(ret));
		}
		else
		{
			printf("Testing \"%s\"\n", argv[optind]);
			waitpid(test(), &ret, 0);
		}
		if(ret)
		{
			fprintf(stderr, "Testing failed! Check \"output.log\" for error output.\n");
			printf("test exit status: %d\n", WEXITSTATUS(ret));
		}
		chdir(base);
		printf("Press Enter to continue");
		getchar();
	} while(++optind < argc);
	free(base);
	return 0;
}

void parseargs(int argc, char ***argv)
{
	int n = 0;
	while((n = getopt(argc, *argv, "lt:m:")) != -1)
	{
		switch(n)
		{
		case 'l':
			optflags |= LOGGING;
			break;
		case 'm':
			optflags |= MAKEARG;
			makefile = optarg;
			realpath(makefile, makefilepath);
			break;
		case 't':
			optflags |= TESTARG;
			testfile = optarg;
			break;
		}
	}
}

pid_t make()
{
	pid_t maker = vfork();
	int fd;
	if(!maker)
	{
		if(optflags & LOGGING)
		{
			if((fd = open("make.log", O_WRONLY | O_CREAT, 0644)) == -1)
			{
				perror("shit.\n");
				_exit(1);
			}
			ftruncate(fd, 0); //Erase Old Content
			if((dup2(fd, 1) | dup2(fd,2)) < 0)
				_exit(2);
			close(fd);
		}
		if(optflags & MAKEARG)
		{
			execlp("make", "make", "-f", makefilepath, NULL);
		}
		else
			fprintf(stderr, "No template makefile provided!\n");
		_exit(3);
	}
	if(maker < 0)
		perror("vfork!");
	return maker;

}
pid_t test()
{
	pid_t tester = vfork();
	int fd;
	if(!tester)
	{
		if(optflags & LOGGING)
		{
			if((fd = open("test.log", O_WRONLY | O_CREAT, 0644)) == -1)
			{
				perror("shit.\n");
				_exit(1);
			}
			ftruncate(fd, 0); //Erase Old Content
			if((dup2(fd, 1) | dup2(fd,2)) < 0)
				_exit(2);
		}
		execl("./tst", "./tst", NULL);
		_exit(3);
	}
	if(tester < 0)
		perror("vfork!");
	return tester;
}
