// File: main.c
// Author: iBug

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "input.h"
#include "global.h"
#include "builtins.h"
#include "parsing.h"
#include "variables.h"

#define MAX_LEN 8192
#define MAX_ARGS 32
#define MAX_ARG_LEN 256

#ifdef D
#define DEBUG(a...) fprintf(stderr, "DEBUG: " a)
#else
#define DEBUG(a...)
#endif

char prompt[MAX_LEN], cmdline[MAX_CMD_LEN];
char args[MAX_ARGS][MAX_ARG_LEN];

char *argv[MAX_ARGS + 1];
int cmdlen, argc, argcount;

int main(int _argc, char * const * _argv) {
    char *s;
    int i, j;
    int is_pipe, redir_mode, rredir = 0, wredir = 0, pipefd[2];

    while (1) {
        char *cmd = get_input(cmdline, 1);
        if (!cmd) {
            if (isatty(STDIN_FILENO) && isatty(STDERR_FILENO))
                fprintf(stderr, "exit\n");
            exit(0);
        }
        cmdlen = strlen(cmd);

        // Split arguments
        i = 0;
        while (i < cmdlen) {
            is_pipe = 0, redir_mode = 0, argc = 0, argcount = 0;
            while (i < cmdlen) {
                // Skip all unwanted stuff
                while (cmd[i] <= ' ' || cmd[i] == '\x7F') {
                    cmd[i] = 0;
                    i++;
                }

                // Make a pointer to all following non-control characters
                int prev_redir_mode = redir_mode, x;
                redir_mode = 0;
                char context = 0, // Handle quotes
                     *parg = args[argcount];
                s = parg;
                for (x = 0; i < cmdlen ; i++) {
                    if (context == 0) {
                        if (cmd[i] <= ' ' || cmd[i] == '\x7F') {
                            parg[x++] = 0;
                            break;
                        } else if (cmd[i] == '\"') {
                            context = '\"';
                            continue;
                        } else if (cmd[i] == '\'') {
                            context = '\'';
                            continue;
                        } else if (cmd[i] == '\\') {
                            char ch;
                            i += escape_char(&ch, cmd + i + 1);
                            parg[x++] = ch;
                        } else if (cmd[i] == '|') {
                            is_pipe = 1;
                            cmd[i] = 0;
                            break;
                        } else if (cmd[i] == '<') {
                            if (cmd[i + 1] == '<') {
                                if (cmd[i + 2] == '<') {
                                    redir_mode = 's'; // Single-line string
                                    i += 3;
                                } else {
                                    redir_mode = 'S'; // Multi-line string
                                    i += 2;
                                }
                            } else {
                                redir_mode = 'r';
                                i++;
                            }
                            break;
                        } else if (cmd[i] == '>') {
                            if (cmd[i + 1] == '>') {
                                redir_mode = 'a';
                                i += 2;
                            } else {
                                redir_mode = 'w';
                                i++;
                            }
                            break;
                        } else if (cmd[i] == '$') {
                            char varname[MAX_VAR_NAME] = {};
                            const char *varvalue = NULL;
                            int j = 0, brace = 0; i += 1;
                            if (cmd[i] == '{') {
                                brace = 1;
                                i += 1;
                            }
                            for (;j < MAX_VAR_NAME; i++, j++) {
                                if ((cmd[i] >= 'A' && cmd[i] <= 'Z') ||
                                    (cmd[i] >= 'a' && cmd[i] <= 'z') ||
                                    (cmd[i] >= '0' && cmd[i] <= '9') ||
                                    cmd[i] == '_') {
                                    varname[j] = cmd[i];
                                } else if (brace) {
                                    if (cmd[i] == '}') break;
                                } else break;
                            }
                            varvalue = get_variable(varname);
                            if (varvalue) {
                                strncpy(parg + x, varvalue, MAX_ARG_LEN - x - 1);
                                parg[MAX_ARG_LEN - 1] = 0;
                                x += strlen(parg + x);
                            }
                            if (!brace)
                                i--;
                            continue;
                        } else if (cmd[i] == '~' && x == 0) {
                            strncpy(parg, getenv("HOME"), MAX_PATH);
                            x = strlen(parg);
                        } else {
                            parg[x++] = cmd[i];
                        }
                    } else if (context == '\"') {
                        if (cmd[i] == '\"') {
                            context = 0;
                            continue;
                        } else if (cmd[i] == '\\') {
                            char ch;
                            i += escape_char(&ch, cmd + i + 1);
                            parg[x++] = ch;
                        } else if (cmd[i] == '$') {
                            char varname[MAX_VAR_NAME] = {};
                            const char *varvalue;
                            int j = 0; i += 1;
                            for (;j < MAX_VAR_NAME; i++, j++) {
                                if ((cmd[i] >= 'A' && cmd[i] <= 'Z') ||
                                    (cmd[i] >= 'a' && cmd[i] <= 'z') ||
                                    (cmd[i] >= '0' && cmd[i] <= '9') ||
                                    cmd[i] == '_') {
                                    varname[j] = cmd[i];
                                } else break;
                            }
                            varvalue = get_variable(varname);
                            if (varvalue) {
                                strncpy(parg + x, varvalue, MAX_ARG_LEN - x - 1);
                                parg[MAX_ARG_LEN - 1] = 0;
                                x += strlen(parg + x);
                            }
                            i--;
                            continue;
                        } else {
                            parg[x++] = cmd[i];
                        }
                    } else if (context == '\'') {
                        if (cmd[i] == '\'')
                            context = 0;
                        else
                            parg[x++] = cmd[i];
                    } else {
                        // Unrecognized "context"
                        context = 0;
                        i--;
                    }
                }
                parg[x] = 0; // Terminate the string

                if (*s) { // Prevent empty stuff
                    if (prev_redir_mode == 'r') {
                        if (rredir > 0)
                            close(rredir); // Avoid jamming
                        cmd[i] = 0;
                        FILE *fp = fopen(s, "r");
                        DEBUG("%x: %s\n", fp, s);
                        rredir = fileno(fp);
                        continue;
                    } else if (prev_redir_mode == 's') {
                        // Single-line string
                        char tmp[64] = "/tmp/ish-XXXXXX";
                        int tmp_fd = mkstemp(tmp);
                        FILE *fp = fdopen(tmp_fd, "w");
                        // Write data
                        fputs(s, fp);
                        fputc('\n', fp);
                        fclose(fp);

                        // Get back written data
                        fp = fopen(tmp, "r");
                        rredir = fileno(fp);
                    } else if (prev_redir_mode == 'S') {
                        // Multi-line string, placeholder
                        char tmp[64] = "/tmp/ish-XXXXXX";
                        int tmp_fd = mkstemp(tmp);
                        FILE *fp = fdopen(tmp_fd, "w");
                        // Write data
                        char *buf = malloc(sizeof(char) * MAX_CMD_LEN),
                             *buf2 = malloc(sizeof(char) * 2 * MAX_CMD_LEN);
                        while (1) {
                            get_input(buf, 2);
                            if (!strcmp(buf, s)) // String end
                                break;
                            int buflen = strlen(buf);
                            // Process input buffer to expand escapes and variables
                            for (int i = 0, j = 0; i < buflen;) {
                                if (buf[i] == '\\') {
                                    // ???
                                } else if (buf[i] == '$') {
                                    // ???
                                } else {
                                    buf2[j++] = buf[i++];
                                }
                            }
                            fputs(buf2, fp);
                            fputc('\n', fp);
                        }
                        free(buf);
                        free(buf2);
                        fclose(fp);

                        // Get back written data
                        fp = fopen(tmp, "r");
                        rredir = fileno(fp);
                    } else if (prev_redir_mode == 'w') {
                        if (wredir > 0)
                            close(wredir);
                        cmd[i] = 0;
                        FILE *fp = fopen(s, "w");
                        wredir = fileno(fp);
                        continue;
                    } else if (prev_redir_mode == 'a') {
                        if (wredir > 0)
                            close(wredir);
                        cmd[i] = 0;
                        FILE *fp = fopen(s, "a");
                        wredir = fileno(fp);
                        continue;
                    } else {
                        argv[argcount++] = s;
                    }
                }

                // Get ready to execute!
                if (cmd[i] == 0) {
                    break;
                }
            }
            if (argcount == 0) {
                printf("No command\n");
                continue;
            }
            argv[argcount] = NULL; // The last pointer of ARGV must be NULL
            DEBUG("argcount = %d\n", argcount);
            for (j = 0; j < argcount; j++)
                DEBUG("$%d = %s\n", j, argv[j]);

            // Check for builtin commands
            if (process_builtin(argcount, argv))
                continue;

            // Handle pipes
            pipefd[0] = pipefd[1] = 0;
            if (is_pipe) {
                if (pipe(pipefd) == -1)
                    fprintf(stderr, "pipe: %s\n", strerror(errno));
                DEBUG("pipe: %d -> %d\n", pipefd[1], pipefd[0]);
                wredir = pipefd[1];
            }

            // Execute the command
            // If the command does not end with a pipe, use fork(2)
            pid_t fork_pid = fork();

            if (fork_pid) {
                // Parent
                // Clear the pipe record
                if (rredir > 0) {
                    close(rredir); // Shut down the old pipe
                    rredir = 0;
                }
                if (is_pipe) {
                    rredir = pipefd[0]; // Record the new pipe for next loop
                }
                if (wredir > 0) {
                    close(wredir);
                    wredir = 0;
                }

                // If the last command has a pipe, don't wait
                if (!is_pipe) {
                    // Wait for the child to complete
                    int status;
                    waitpid(fork_pid, &status, 0);
                    DEBUG("Child exit code: %d\n", WEXITSTATUS(status));
                    if (WEXITSTATUS(status) != 0) {
                        last_ecode = WEXITSTATUS(status);
                        // fprintf(stderr, "%d exited: %d\n", fork_pid, WEXITSTATUS(status));
                    }
                }
            }
            else {
                // Child - Go execve
                if (rredir > 0) {
                    // The last command has an open pipe, connect it with stdin:
                    dup2(rredir, 0);
                    close(rredir);
                }
                if (wredir > 0) {
                    // The current command has an outgoing redirection
                    dup2(wredir, 1);
                    close(wredir);
                }
                execvp(argv[0], argv);

                // Normally unreachable - something's wrong
                fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
                exit(127);
            }
        }
    }
    return 0;
}
