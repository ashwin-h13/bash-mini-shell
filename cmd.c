// SPDX-License-Identifier: BSD-3-Clause

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <unistd.h>

#include "cmd.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>


#define READ		0
#define WRITE		1

/**
 * Internal change-directory command.
 */
static int shell_cd(word_t *dir)
{
	/* TODO: Execute cd. */
	if (dir == NULL){
		return 0;
	}

	// concat helper function syntax: char *get_word(word_t *s);
	// get path from word
	char *path = get_word(dir);

	// call chdir to change dir
	int ret = chdir(path);

	// zero on success
	if (ret == 0){
		return 0;
	}

	else{
		return -1;
	}

}

/**
 * Internal exit/quit command.
 */
static int shell_exit(void)
{
	/* TODO: Execute exit/quit. */

	return SHELL_EXIT; /* TODO: Replace with actual exit code. */
}

static void cd_redirect(simple_command_t *s){

	if (s == NULL){
		return;
	}

	if (s->in != NULL)
	{
		//  get path
		char *path = get_word(s->in);

		// printf("GOT path for in red\n");
		//  open file with path to get file descriptor
		int fd = open(path, O_RDONLY);


		// redirect input from std in to the file: dup2(int oldfd, int newfd)
		dup2(fd, STDIN_FILENO);

		// close file
		int ret = close(fd);

		free(path);

		// close failed
		if (ret == -1)
		{
			// return -1;
		}
	}

	// both out and err redirect
	if ((s->out != NULL) && (s->err != NULL))
	{
		// get path
		char *path = get_word(s->out);

		// get flags
		int flags = s->io_flags;

		// init fd outside of if
		int fd;

		//  based on flags, have diff flags for open()
		if (flags == IO_OUT_APPEND || flags == IO_ERR_APPEND)
		{
		
			//  open file with path to get file descriptor
			fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0777);
		}
		else
		{
			
			//  open file with path to get file descriptor
			fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
		}

		// close file
		int ret = close(fd);

		free(path);

		// close failed
		if (ret == -1)
		{
			return -1;
		}
	}

	else if ((s->out != NULL))
	{
		

		// get path
		char *path = get_word(s->out);
		// get flags
		int flags = s->io_flags;
		
		// init fd outside of if
		int fd;
		

		//  based on flags, have diff flags for open()
		if (flags == IO_OUT_APPEND || flags == IO_ERR_APPEND)
		{
			
			//  open file with path to get file descriptor
			fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0777);
			
		}
		else
		{
			//  open file with path to get file descriptor
			fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			
		}

		// close file
		close(fd);
		

		free(path);

		return;

	}

	else if (s->err != NULL)
	{

		//  get path
		char *path = get_word(s->err);


		// get flags
		int flags = s->io_flags;


		// init fd outside of if blocks
		int fd;


		// based on flags, have diff flags for open(), and get fd

		if (flags == IO_OUT_APPEND || flags == IO_ERR_APPEND)
		{
			
			fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0777);
		}

		else
		{
			
			fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
		}


		// close file
		int ret = close(fd);

		free(path);

		// close failed
		if (ret == -1)
		{
			return -1;
		}
	}

	return 0;
}

/**
 * Parse a simple command (internal, environment variable assignment,
 * external command).
 */
static int parse_simple(simple_command_t *s, int level, command_t *father)
{
	/* TODO: Sanity checks. */
	if (s == NULL){
		
		return -1;
	}

	/* TODO: If builtin command (cd or exit), execute the command. */
	
	// call get word to see what command is, s->verb becomes word_t type
	char *cmd = get_word(s->verb);


	// see if its cd
	if (strcmp(cmd, "cd") == 0){
		// perform redirections
		cd_redirect(s);

		// perform the cd
		int ret = shell_cd(s->params);

		// zero on success
		if (ret == 0)
		{
			return 0;
		}
		else
		{
			return -1;
		}
		}
		
	

	// see if its exit
	if (strcmp(cmd, "exit") == 0){
		//call shell exit
		return shell_exit();
	}

	// see if its quit
	if (strcmp(cmd, "quit") == 0){
		//call shell exit
		return shell_exit();
	}

	/* TODO: If variable assignment, execute the assignment and return
	 * the exit status. 
	 */

	// if next part is "=" then doing var assignment
	
	// make sure next part isnt null
	if (s->verb->next_part != NULL){
		// get string of next part
		char *next_part = (s->verb->next_part->string);

		if (strcmp(next_part, "=") == 0){
			//perform var assignment: setenv(const char *name, const char *value, int overwrite)
			// overwrite ≠ 0 replaces existing variable

			// get name and value: "name=value"
			char *name = (s->verb->string);
			char *value = get_word(s->verb->next_part->next_part);

			return setenv(name, value, 1);
		}

	}

	

	/* TODO: If external command:
	 *   1. Fork new process
	 *     2c. Perform redirections in child
	 *     3c. Load executable in child
	 *   2. Wait for child
	 *   3. Return exit status
	 */


	// fork
	pid_t pid = fork();


	// fork failed
	if (pid < 0){
		return -1;
	}

	// child process
	if (pid == 0){
		// do redirections in child
		if (s->in != NULL){
			// get path
			char *path = get_word(s->in);

			// open file with path to get file descriptor
			int fd = open(path, O_RDONLY);

			// redirect input from std in to the file: dup2(int oldfd, int newfd)
			dup2(fd, STDIN_FILENO);


			// close file
			int ret = close(fd);

			free(path);

			// close failed
			if (ret == -1){
				return -1;
			}
		}
	

		// both out and err redirect
		if ((s->out != NULL) && (s->err != NULL)){

			
			// get paths
			char *outpath = get_word(s->out);
			char *errpath = get_word(s->err);

			// case where outpath = errpath
			if (strcmp(outpath, errpath) == 0)
			{
				// get flags
				int flags = s->io_flags;

				// init fd outside of if
				int fd;

				// based on flags, have diff flags for open()

				if (flags == IO_OUT_APPEND || flags == IO_ERR_APPEND)
				{
					// open file with path to get file descriptor
					fd = open(outpath, O_WRONLY | O_CREAT | O_APPEND, 0777);
				}
				else
				{
					// open file with path to get file descriptor
					fd = open(outpath, O_WRONLY | O_CREAT | O_TRUNC, 0777);
				}

				// redirect output to file instead of std out
				dup2(fd, STDOUT_FILENO);

				// redirect output to file instead of std error
				dup2(fd, STDERR_FILENO);

				// close file
				int ret = close(fd);

				free(outpath);

				// close failed
				if (ret == -1)
				{
					return -1;
				}
			}
			// outpath != errpath
			else{

				// get flags
				int flags = s->io_flags;

				// init fd outside of if
				int outfd;
				int errfd;

				// based on flags, have diff flags for open()

				if (flags == IO_OUT_APPEND || flags == IO_ERR_APPEND)
				{
					// open file with path to get file descriptor
					outfd = open(outpath, O_WRONLY | O_CREAT | O_APPEND, 0777);
					errfd = open(errpath, O_WRONLY | O_CREAT | O_APPEND, 0777);
				}
				else
				{
					// open file with path to get file descriptor
					outfd = open(outpath, O_WRONLY | O_CREAT | O_TRUNC, 0777);
					errfd = open(errpath, O_WRONLY | O_CREAT | O_TRUNC, 0777);
				}

				// redirect output to file instead of std out
				dup2(outfd, STDOUT_FILENO);

				// redirect output to file instead of std error
				dup2(errfd, STDERR_FILENO);

				// close files
				close(outfd);
				close(errfd);

				free(outpath);
				free(errpath);
			}
		}

		else if ((s->out != NULL)){

	
			// get path
			char *path = get_word(s->out);

			// get flags
			int flags = s->io_flags;

			// init fd outside of if
			int fd;

			
			// based on flags, have diff flags for open()
			if (flags == IO_OUT_APPEND || flags == IO_ERR_APPEND){
				// open file with path to get file descriptor
				fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0777);
			}
			else{
				// open file with path to get file descriptor
				fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
			}

			// redirect output to file instead of std out
			dup2(fd, STDOUT_FILENO);

			// close file
			int ret = close(fd);

			free(path);

			// close failed
			if (ret == -1){
				return -1;
			}
		}

		else if (s->err != NULL){
			// get path
			char *path = get_word(s->err);

			// get flags
			int flags = s->io_flags;


			// init fd outside of if blocks
			int fd;


			// based on flags, have diff flags for open(), and get fd
			
			if (flags == IO_OUT_APPEND || flags == IO_ERR_APPEND){
				fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0777);
			}
		
			else {
				fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
			}


			// redirect output on error to file instead of std out
			dup2(fd, STDERR_FILENO);

			// close file
			int ret = close(fd);

			free(path);

			// close failed
			if (ret == -1){
				return -1;
			}

		}
	
		// load executables in child, use execvp to replace current process with file
		// get executable name and args
		char *file = get_word(s->verb);
		int size;
		char **args = get_argv(s, &size);
		
		int ret = execvp(file, args);
		// execvp failed
		if (ret == -1){
			printf("Execution failed for '%s'\n", file);
			exit(1);
			return -1;
		}
	}

	// parent process
	if (pid > 0){
		// wait for child, use waitpid()
		int status;
	
		waitpid(pid, &status, 0);

		// use weexitstatus() to get childs exit code and return
		return WEXITSTATUS(status);

	}


}

/**
 * Process two commands in parallel, by creating two children.
 */
static int run_in_parallel(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	/* TODO: Execute cmd1 and cmd2 simultaneously. */
	
	pid_t pid1 = fork();
	
	// fork failed
	if (pid1 < 0){
		return -1;
	}

	// First Child, run cmd1
	else if (pid1 == 0){
		int ret_val = parse_command(cmd1, level, father);
		exit(ret_val);
	}

	// parent of child 1
	else if (pid1 > 0){
		// create second child
		pid_t pid2 = fork();

		// fork failed
		if (pid2 < 0){
			return -1;
		}

		// second child, run cmd2
		else if (pid2 == 0){
			int ret_val = parse_command(cmd2, level, father);
			exit(ret_val);

		}

		// parent of both commands
		else if (pid2 > 0){
			// here wait for both commands to finish
			int status1;
			int status2;

			waitpid(pid1, &status1, 0);
			waitpid(pid2, &status2, 0);
	
			// return exit status of second one
			return WEXITSTATUS(status2);

		}

	}

	 
}

/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2).
 */
static int run_on_pipe(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	/* TODO: Redirect the output of cmd1 to the input of cmd2. */
	// holds read/write fd, pipefd[0] is read, pipefd[1] is write
	int pipefd[2];

	// create pipe, see if pipe failed
	if (pipe(pipefd) == -1){
		return -1;
	}

	// fork
	pid_t pid1 = fork();

	// fork failed
	if (pid1 < 0){
		return -1;
	}

	// First Child, run cmd1
	else if (pid1 == 0){
		// close read end of pipe
		close(pipefd[0]);

		// dup2 to write to pipe instead of std out
		dup2(pipefd[1], STDOUT_FILENO);

		// run cmd1
		int ret_val = parse_command(cmd1, level, father);

		// close write end of pipe
		close(pipefd[1]);

		// returns 0 on success
		if (ret_val == 0){
			exit(0);
		}
		else{
			exit(1);
		}
		
	}

	// parent of child 1
	else if (pid1 > 0){

		// create second child
		pid_t pid2 = fork();

		// fork failed
		if (pid2 < 0){
			return -1;
		}

		// second child, 
		else if (pid2 == 0){
			// close write end of pipe
			close(pipefd[1]);

			// dup2 to read from pipe instead of std in
			dup2(pipefd[0], STDIN_FILENO);

			// run cmd 2
			int ret_val = parse_command(cmd2, level, father);

			// close read end of pipe
			close(pipefd[0]);

			if (ret_val == 0){
				exit(0);
			}
			else{
				exit(1);
			}

			
		}

		// parent of both commands
		else if (pid2 > 0){

			close(pipefd[0]);
			close(pipefd[1]);

			// here wait for both commands to finish
			int status1;
			int status2;

			int wpid1 = waitpid(pid1, &status1, 0);
			int wpid2 = waitpid(pid2, &status2, 0);
	

			// see how process exited
			if (WIFEXITED(status2)){
				return WEXITSTATUS(status2);
			}
			else if (WIFSIGNALED(status1)){
				return WTERMSIG(status1);
			}
			else if (WIFSIGNALED(status2)){
				return WTERMSIG(status2);
			}

		}

	}

}

/**
 * Parse and execute a command.
 */
int parse_command(command_t *c, int level, command_t *father)
{
	/* TODO: sanity checks */
	

	if (c->op == OP_NONE) {
		/* TODO: Execute a simple command. */
		int ret_val = parse_simple(c->scmd, level, father);
		return ret_val;
		
	}

	switch (c->op) {
	case OP_SEQUENTIAL:
		/* TODO: Execute the commands one after the other. */
		//execute cmd1 then cmd2
		int ret_val1 = parse_command(c->cmd1, level, father);
		int ret_val2 = parse_command(c->cmd2, level, father);

		// see if both succeeded or not
		if ((ret_val1 == 0) && (ret_val2 == 0)){
			return 0;
		}
		else{
			return -1;
		}

	case OP_PARALLEL:
		/* TODO: Execute the commands simultaneously. */
		// call run in parallel function
		int ret_val = run_in_parallel(c->cmd1, c->cmd2, level, father);
		return ret_val;

	case OP_CONDITIONAL_NZERO:
		/* TODO: Execute the second command only if the first one
		 * returns non zero. ||
		 */
		//execute cmd1 then cmd2 only if cmd1 returns zonzero 
		int condn_ret_val = parse_command(c->cmd1, level, father);
		int condn_ret_val2 = -1;
		
		if (condn_ret_val != 0)
		{
			condn_ret_val2 = parse_command(c->cmd2, level, father);
		}

		// logic to mimic x || y
		if (condn_ret_val == 0 || condn_ret_val2 == 0){
			return 0;
		}
		else{
			return -1;
		}

		

	case OP_CONDITIONAL_ZERO:
		/* TODO: Execute the second command only if the first one
		 * returns zero. &&
		 */
	
		//execute cmd1 then cmd2 only if cmd1 returns zero
		int cond_ret_val = parse_command(c->cmd1, level, father);
		int cond_ret_val2 = -1;
		
		if (cond_ret_val == 0){
			cond_ret_val2 = parse_command(c->cmd2, level, father);
			
		}

		// logic to mimic x && y
		if (cond_ret_val == 0 && cond_ret_val2 == 0){
			return 0;
		}
		else{
			return -1;
		}

	case OP_PIPE:
		/* TODO: Redirect the output of the first command to the
		 * input of the second.
		 */
		
		// call run on pipe function
		int pipe_ret_val = run_on_pipe(c->cmd1, c->cmd2, level, father);
		
		return pipe_ret_val;

	default:
		return SHELL_EXIT;
	}

}
