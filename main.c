#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define DEBUG 1
#undef DEBUG

pid_t make();
pid_t test();

int main(int argc, char **argv)
{
	int ret = 0;
	char *base = getcwd(NULL,0);
	do
	{
		if(--argc)
			chdir(argv[argc]);
		printf("Making the project...\n");
		waitpid(make(), &ret, 0);
		#ifdef DEBUG
		printf("parent!\n");
		#endif
		if(ret)
			fprintf(stderr, "Make failed! Check \"make.log\" for error output.\n");
		printf("make exit status: %d\n", WEXITSTATUS(ret));
		waitpid(test(), &ret, 0);
		if(ret)
			fprintf(stderr, "Testing failed! Check \"output.log\" for error output.\n");
		printf("test exit status: %d\n", WEXITSTATUS(ret));
		chdir(base);
	} while(argc > 1);
	free(base);
	return 0;
}
pid_t make()
{
	pid_t maker = vfork();
	int fd;
	if(!maker)
	{
		#ifdef DEBUG
		printf("this is a child\n");
		#endif
		if((fd = open("make.log", O_WRONLY | O_CREAT, 0644)) == -1)
		{
			perror("shit.\n");
			_exit(1);
		}
		ftruncate(fd, 0); //Erase Old Content
		if((dup2(fd, 1) | dup2(fd,2)) < 0)
			_exit(2);
		close(fd);
		execlp("make", "make", NULL);
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
		#ifdef DEBUG
		printf("this is a child\n");
		#endif
		if((fd = open("test.log", O_WRONLY | O_CREAT, 0644)) == -1)
		{
			perror("shit.\n");
			_exit(1);
		}
		ftruncate(fd, 0); //Erase Old Content
		if((dup2(fd, 1) | dup2(fd,2)) < 0)
			_exit(2);
		execl("./tst", "./tst", NULL);
		_exit(3);
	}
	if(tester < 0)
		perror("vfork!");
	return tester;
}
