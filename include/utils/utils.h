#ifndef _IPC_UTILS_H_
#define _IPC_UTILS_H_

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <memory.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>


#define MAXLINE 4096 /* max line length */
// #define LINE_BUF_MAXLEN 1024

#define DEFAULT_SIZE 4096
#define DEFAULT_COUNT 1000

#define PROMPT_SIZE 16

#define EOT                                                                    \
  4 // in telecommunication, EOT means "End-of-Transmission character".
#define DEL 127 // ascii code of backspace

#define required_argument 1

typedef struct arguments {
  int msg_size;
  int msg_count; // for test
} arguments_t;

typedef struct t_line_buffer {
  char buf[MAXLINE]; // line of text to read
  char prompt[PROMPT_SIZE];
  int pos;        // position in buf
  int line_ready; // buf is ready for print
                  // int end_of_input;
} line_buf_t;

void parse_arguments(arguments_t *args, int argc, char **argv);

void err_exit(int, const char *, ...) __attribute__((noreturn));
void err_sys(const char *, ...) __attribute__((noreturn));

// === Use for non-canonical (just-in-time) terminal output ===
struct termios *active_noncanonical_terminal_mode(struct termios *oldt_p);
void reset_terminal_mode(struct termios *oldt_p);

void linebuf_reset(line_buf_t *line_buf);
void linebuf_set_prompt(line_buf_t *line_buf, char *prompt);
void linebuf_get_char(line_buf_t *line_buf);
void linebuf_print(line_buf_t *line_buf, char *fmt, ...);
// =============================================================


// === file ===
int set_io_flag(int fd, int flag);
// ============


#endif
