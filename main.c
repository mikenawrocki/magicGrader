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

//#define LOGGING (1 << 0)
//#define MAKEARG (1 << 1)
//#define TESTARG (1 << 2)

struct flags {
	unsigned int logging : 1;
	unsigned int makearg : 1;
	unsigned int testarg : 1;
	char makefilepath[PATH_MAX];
	char testfilepath[PATH_MAX];
} FLAGS = { .logging = 0, .makearg = 0, .testarg = 0} ;


pid_t make();
pid_t test();
void parseargs(int argc, char ***argv);

static unsigned char optflags;
//char makefilepath[PATH_MAX];
//char testfilepath[PATH_MAX];


int main(int argc, char **argv)
{
	int ret = 0;
	int i;
	int opts;
	char *base = getcwd(NULL,0);
	parseargs(argc, &argv);
	opts = optind;
	while(opts < argc)
	{
		if(chdir(argv[opts]) != -1)
		{
			ret = 0;
			printf("Making \"%s\"\n", argv[opts]);
			waitpid(make(), &ret, 0);
			if(ret)
			{
				fprintf(stderr, "Make failed!\n");
				printf("make exit status: %d\n", WEXITSTATUS(ret));
			}
			printf("Testing \"%s\"\n", argv[opts]);
			waitpid(test(), &ret, 0);
			if(ret)
			{
				fprintf(stderr, "Testing failed!\n");
				printf("test exit status: %d\n", WEXITSTATUS(ret));
			}
			printf("Press Enter to continue");
			getchar();
		}
		else
			perror(argv[opts]);
		chdir(base);
		opts++;
	} 
	free(base); 
	return 0;
}

void parseargs(int argc, char ***argv)
{
	int n = 0;
	char *makefile;
	char *testfile;
	while((n = getopt(argc, *argv, "lt:m:")) != -1)
	{
		switch(n)
		{
		case 'l':
			//optflags |= LOGGING;
			FLAGS.logging = 1;
			break;
		case 'm':
			//optflags |= MAKEARG;
			FLAGS.makearg = 1;
			makefile = optarg;
			realpath(makefile, FLAGS.makefilepath);
			break;
		case 't':
			//optflags |= TESTARG;
			FLAGS.testarg = 1;
			testfile = optarg;
			realpath(testfile, FLAGS.testfilepath);
			break;
		}
	}
	if(optind == argc)
	{
		fprintf(stderr,"No Directories provided! Aborting!\n");
		exit(-1);
	}
	if(!(FLAGS.makearg))
	{
		fprintf(stderr, "No Makefile template provided. Aborting!\n");
		exit(-2);
	}
}

pid_t make()
{
	pid_t maker = vfork();
	int fd;
	if(!maker)
	{
		//if(optflags & LOGGING)
		if(FLAGS.logging)
		{
			if((fd = open("make.log", O_WRONLY | O_CREAT, 0644)) == -1)
			{
				perror("make.log");
				_exit(1);
			}
			ftruncate(fd, 0); //Erase Old Content
			if((dup2(fd, 1) | dup2(fd,2)) < 0)
				_exit(2);
			close(fd);
		}
		if(FLAGS.makearg)
		{
			execlp("make", "make", "-f", FLAGS.makefilepath, NULL);
		}
		else
			fprintf(stderr, "No template makefile provided!\n");
		_exit(3);
	}
	if(maker < 0)
		perror("in make()");
	return maker;

}
pid_t test()
{
	pid_t tester = vfork();
	int fd,tst;
	if(!tester)
	{
		//if(optflags & LOGGING)
		if(FLAGS.logging)
		{
			if((fd = open("test.log", O_WRONLY | O_CREAT, 0644)) == -1)
			{
				perror("test.log");
				_exit(1);
			}
			ftruncate(fd, 0); //Erase Old Content
			if((dup2(fd, 1) | dup2(fd,2)) < 0)
			{
				perror("stdin and stderr");
				_exit(2);
			}
		}
		//if(optflags & TESTARG)
		if(FLAGS.testarg)
		{
			if((tst = open(FLAGS.testfilepath, O_RDONLY)) == -1)
			{
				perror(FLAGS.testfilepath);
				_exit(1);
			}
			if((dup2(tst, 0)) < 0)
			{
				perror("stdin redirect");
				_exit(2);
			}
		}
		execl("./tst", "./tst", NULL);
		_exit(3);
	}
	if(tester < 0)
		perror("in test()");
	return tester;
}
