#include "utils.h"

static void err_doit(int errnoflag, int error, const char *fmt, va_list ap);

void parse_arguments(arguments_t *args, int argc, char **argv) {
    int long_idx = 0;
    int option;

    optind = 0;

    args->msg_size = DEFAULT_SIZE;
    args->msg_count = DEFAULT_COUNT;

    // clang-format off
    static struct option long_options[] = {
        {"size", required_argument, 0, 's'},
        {"count", required_argument, 0, 'c'},
        {0, 0, 0, 0}
    };
    // clang-format on

    while (1) {
        option = getopt_long(argc, argv, "+:s:c:", long_options, &long_idx);
        switch (option) {
            case -1:
                return;
            case 's':
                // args->msg_size = atoi(optarg);
                args->msg_size = (int)strtol(optarg, NULL, 10);

                break;
            case 'c':
                args->msg_count = (int)strtol(optarg, NULL, 10);
                break;
            default:
                continue;
        }
    }
}

/*
 * Fatal error unrelated to a system call.
 * Error code passed as explict parameter.
 * Print a message and terminate.
 */
void err_exit(int error, const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, error, fmt, ap);
    va_end(ap);
    exit(1);
}

/*
 * Fatal error related to a system call.
 * Print a message and terminate.
 */
void err_sys(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    va_end(ap);
    exit(1);
}

/*
 * Print a message and return to caller.
 * Caller specifies "errnoflag".
 */
static void err_doit(int errnoflag, int error, const char *fmt, va_list ap) {
    char buf[MAXLINE];

    (void)vsnprintf(buf, MAXLINE - 1, fmt, ap);
    if (errnoflag) {
        (void)snprintf(buf + strlen(buf), MAXLINE - strlen(buf) - 1, ": %s", strerror(error));
    }
    strcat(buf, "\n");

    (void)fflush(stdout); /* in case stdout and stderr are the same */
    (void)fputs(buf, stderr);
    (void)fflush(NULL); /* flushes all stdio output streams */
}


// ===================================================================
// non-canonical mode: each character of input is passed to terminal buffer immediately.
struct termios* active_noncanonical_terminal_mode(struct termios* oldt_p) {
    struct termios newt;

    // Turn off output buffering
    (void)setvbuf(stdout, NULL, _IONBF, 0);

    // get terminal settings
    tcgetattr(STDIN_FILENO, oldt_p);
    newt = *oldt_p;

    // turn off return when sees a "\n" or an EOF or an EOL
    newt.c_lflag &= ~(ICANON | ECHO);
    //  newt.c_lflag &= ~(ICANON);

    // set terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    return oldt_p;
}


inline void reset_terminal_mode(struct termios* oldt_p) {
    tcsetattr(STDIN_FILENO, TCSANOW, oldt_p);
}


void linebuf_reset(line_buf_t *line_buf) {
    line_buf->pos = 0;
    line_buf->buf[0] = '\0';
    line_buf->line_ready = 0;
    // line_buf->end_of_input = 0;
}


void linebuf_set_prompt(line_buf_t *line_buf, char *prompt) {
    strncpy(line_buf->prompt, prompt, PROMPT_SIZE);
}


// Work in non-canonical terminal mode. Read a character from stdin.
void linebuf_get_char(line_buf_t *line_buf) {
    int c = fgetc(stdin);
    if ((c == '\n' || c == EOT || c == EOF) && line_buf->pos > 0) {  // input complete
        line_buf->buf[line_buf->pos] = '\0';
        line_buf->line_ready = 1;
    } else if ((c == '\b' || c == '\n' || c == DEL) && line_buf->pos == 0) {
        // ignore enter, backspace without input
    } else if ((c == '\b' || c == DEL) && line_buf->pos > 0) { // backspace or delete
        line_buf->pos = line_buf->pos - 1;
        line_buf->buf[line_buf->pos] = '\0';
        // erase last character
        write(STDOUT_FILENO, "\b \b", 3);  
    } else if (c != EOF && c != EOT && line_buf->pos < MAXLINE - 1) {  // normal character
        line_buf->buf[line_buf->pos] = (char)c;
        line_buf->pos++;
        line_buf->buf[line_buf->pos] = '\0';
        (void)fputc(c, stdout);
    }
}

// Make the input at the bottom.
// Keep the current input at the input position (stored in line_buf).
void linebuf_print(line_buf_t *line_buf, char *fmt, ...) {
    char output[MAXLINE * 2];
    int maxlen = MAXLINE * 2;
    int off = 0;

    // clear the current active line, that is the input line.
    off += snprintf(output, maxlen, "\33[2K\r");
    
    // new message
    va_list args;
    va_start(args, fmt);
    off += vsnprintf(output + off, MAXLINE, fmt, args);
    va_end(args);
    
    // reinput the current input which was cleared by the previous "\33[2K\r" input.
    off += snprintf(output + off, maxlen - off, "%s", line_buf->prompt);
    off += snprintf(output + off, maxlen - off, "%s", line_buf->buf);

    write(STDOUT_FILENO, output, off);
}


// ===================================================================

int set_io_flag(int fd, int flag) {
    int old_flag = fcntl(fd, F_GETFL, 0);
    if (old_flag < 0) {
        return -1;
    }

    if (fcntl(fd, F_SETFL, old_flag | flag) < 0) {
        return -1;
    }

    return 0;
}