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
#include "magicgrader.h"
static struct flags FLAGS;
pid_t maker, tester, errorer;
int main(int argc, char **argv)
{
	int ret = 0;
	int i;
	char *base = getcwd(NULL,0);
	parseargs(argc, &argv);
	struct sigaction new_action, old_action;
	new_action.sa_handler = murder;
	sigemptyset(&new_action.sa_mask);
	new_action.sa_flags = 0;
	sigaction(SIGINT, &new_action, NULL); 

	while(optind < argc)
	{
		if(chdir(argv[optind]) != -1)
		{
			ret = 0;
			printf("Making \"%s\"\n", argv[optind]);
			maker = make();
			waitpid(maker, &ret, 0);
			maker = 0;
			if(ret)
			{
				fprintf(stderr, "Make failed!\n");
				printf("make exit status: %d\n", WEXITSTATUS(ret));
			}
			printf("Testing \"%s\"\n", argv[optind]);
			tester = test();
			waitpid(tester, &ret, 0);
			tester = 0;
			if(ret)
			{
				fprintf(stderr, "Testing failed!\n");
				printf("test exit status: %d\n", WEXITSTATUS(ret));
			}
			if(FLAGS.errorarg)
			{
				printf("Testing \"%s\"\n", argv[optind]);
				errorer = error();
				waitpid(tester, &ret, 0);
				tester = 0;
				if(ret)
				{
					fprintf(stderr, "Testing failed!\n");
					printf("test exit status: %d\n", WEXITSTATUS(ret));
				}
			}
			errorer = 0;
		}
		else
			perror(argv[optind]);
		chdir(base);
		optind++;
		printf("\nPress Enter to continue");
		getchar();
	} 
	free(base); 
	return 0;
}

void parseargs(int argc, char ***argv)
{
	int n = 0;
	char *makefile;
	char *testfile;
	char *errorfile;
	while((n = getopt(argc, *argv, "lt:m:e:")) != -1)
	{
		switch(n)
		{
		case 'l':
			FLAGS.logging = 1;
			break;
		case 'm':
			FLAGS.makearg = 1;
			makefile = optarg;
			realpath(makefile, FLAGS.makefilepath);
			break;
		case 't':
			FLAGS.testarg = 1;
			testfile = optarg;
			realpath(testfile, FLAGS.testfilepath);
			break;
		case 'e':
			FLAGS.errorarg = 1;
			errorfile = optarg;
			realpath(errorfile, FLAGS.errorfilepath);
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
pid_t error()
{
	pid_t errorpid = vfork();
	int fd,tst;
	if(!errorpid)
	{
		if(FLAGS.logging)
		{
			if((fd = open("error.log", O_WRONLY | O_CREAT, 0644)) == -1)
			{
				perror("error.log");
				_exit(1);
			}
			ftruncate(fd, 0); //Erase Old Content
			if((dup2(fd, 1) | dup2(fd,2)) < 0)
			{
				perror("stdin and stderr");
				_exit(2);
			}
		}
		if(FLAGS.errorarg)
		{
			if((tst = open(FLAGS.errorfilepath, O_RDONLY)) == -1)
			{
				perror(FLAGS.errorfilepath);
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
	if(errorpid< 0)
		perror("in errorer()");
	return errorpid;
}
