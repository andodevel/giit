/*
 * giit - a git launcher
 * Insprired by gti and sl :D
 * Not support for MS Windows.
 *
 */

#define _POSIX_C_SOURCE 199309L
#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE

#define GIT_NAME "git"

#ifndef GIIT_SPEED
#define GIIT_SPEED 1000
#endif

#ifndef ANSI_COLOR_RED
#define ANSI_COLOR_RED "\x1b[31m"
#endif

#ifndef ANSI_COLOR_GREEN
#define ANSI_COLOR_GREEN "\x1b[32m"
#endif

#ifndef ANSI_COLOR_YELLOW
#define ANSI_COLOR_YELLOW "\x1b[33m"
#endif

#ifndef ANSI_COLOR_BLUE
#define ANSI_COLOR_BLUE "\x1b[34m"
#endif

#ifndef ANSI_COLOR_MAGENTA
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#endif

#ifndef ANSI_COLOR_CYAN
#define ANSI_COLOR_CYAN "\x1b[36m"
#endif

#ifndef ANSI_COLOR_RESET
#define ANSI_COLOR_RESET "\x1b[0m"
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

int term_width(void);
void init_space(void);
void open_term();
void move_to_top(void);
void line_at(int start_x, const char *s);
void clear_drawing(int x);

typedef void (*draw_fn_t)(int x);
void draw_std(int x);
void draw_push(int x);
void draw_pull(int x);
draw_fn_t select_command(int argc, char **argv);

FILE *TERM_FH;
int TERM_WIDTH;
unsigned int FRAME_TIME;
unsigned int PUSH_MOVE[] = {0, 1, 2, 3, 4, 3, 2, 1};
unsigned int LINES_NUMBER = 8;
unsigned int BUFFER_SIZE = 1024;

int main(int argc, char **argv) {
  int i;
  char *git_path;
  char *tmp;
  unsigned int giit_speed;
  draw_fn_t draw_fn;

  tmp = getenv("GIIT_SPEED");
  if (!tmp || sscanf(tmp, "%u", &giit_speed) != 1) {
    giit_speed = GIIT_SPEED;
  }
  open_term();
  TERM_WIDTH = term_width();
  FRAME_TIME = 1000 * 1000 * 10 / (giit_speed + TERM_WIDTH + 1);

  draw_fn = select_command(argc, argv);
  init_space();
  for (i = -20; i < TERM_WIDTH; i++) {
    draw_fn(i);
    clear_drawing(i);
  }
  move_to_top();
  fflush(TERM_FH);

  git_path = getenv("GIT");
  if (git_path) {
    execv(git_path, argv);
  } else {
    execvp(GIT_NAME, argv);
  }
  /* error in exec if we land here */
  perror(GIT_NAME);
  return 1;
}

draw_fn_t select_command(int argc, char **argv) {
  int i;

  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-')
      continue;
    if (!strcmp(argv[i], "push"))
      return draw_push;
    if (!strcmp(argv[i], "pull"))
      return draw_pull;
    break;
  }
  return draw_std;
}

void init_space(void) {
  unsigned int i;
  char buff[BUFFER_SIZE];
  strcpy(buff, "");
  for (i = 0; i < LINES_NUMBER; ++i) {
    strcat(buff, "\n");
  }
  fputs(buff, TERM_FH);
  fflush(TERM_FH);
}

void open_term() {
  TERM_FH = fopen("/dev/tty", "w");
  if (!TERM_FH)
    TERM_FH = stdout;
}

int term_width(void) {
  struct winsize w;
  ioctl(fileno(TERM_FH), TIOCGWINSZ, &w);
  return w.ws_col;
}

void move_to_top(void) { fprintf(TERM_FH, "\033[7A"); }

void move_to_x(int x) { fprintf(TERM_FH, "\033[%dC", x); }

void line_at(int start_x, const char *s) {
  int x;
  size_t i;
  if (start_x > 1)
    move_to_x(start_x);
  for (x = start_x, i = 0; i < strlen(s); x++, i++) {
    if (x > 0 && x < TERM_WIDTH)
      fputc(s[i], TERM_FH);
  }
  fputc('\n', TERM_FH);

  fflush(TERM_FH);
}

void draw_std(int x) {
  move_to_top();
  line_at(x, "   ,---------------.");
  line_at(x, "  /  /``````|``````\\\\");
  line_at(x, " /  /_______|_______\\\\________");
  line_at(x, "|]      giit |'       |        |]");
  if (x % 2) {
    line_at(x, "=  .-:-.    |________|  .-:-.  =");
    line_at(x, " `  -+-  --------------  -+-  '");
    line_at(x, "   '-:-'                '-:-'  ");
  } else {
    line_at(x, "=  .:-:.    |________|  .:-:.  =");
    line_at(x, " `   X   --------------   X   '");
    line_at(x, "   ':-:'                ':-:'  ");
  }

  usleep(FRAME_TIME);
}

void draw_push(int x) {
  move_to_top();
  line_at(x, "   __      ,---------------.");
  line_at(x, "  /--\\   /  /``````|``````\\\\");
  line_at(x, "  \\__/  /  /_______|_______\\\\________");
  line_at(x, "   ||-< |]      giit |'       |        |]");
  if (x % 2) {
    line_at(x, "   ||-< =  .-:-.    |________|  .-:-.  =");
    line_at(x, "   ||    `  -+-  --------------  -+-  '");
    line_at(x, "   ||      '-:-'                '-:-'  ");
  } else {
    line_at(x, "   ||-< =  .:-:.    |________|  .:-:.  =");
    line_at(x, "   /\\    `   X   --------------   X   '");
    line_at(x, "  /  \\     ':-:'                ':-:'  ");
  }

  usleep(FRAME_TIME * 10);
}

void draw_pull(int x) {
  int x1 = PUSH_MOVE[x % 8];
  move_to_top();
  char buff[100];
  line_at(x, "                                       ");
  line_at(x, "                                       ");
  if (x1 == 0) {
    sprintf(buff, "             %s~%s=[,,_,,]:3               ", ANSI_COLOR_RED,
            ANSI_COLOR_RESET);
    line_at(x, buff);
    sprintf(buff, "                                       ");
    line_at(x, buff);
    sprintf(buff, "     %s|%s                                 ",
            ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
    line_at(x, buff);
    sprintf(buff, "   %s--*--%s                               ",
            ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
    line_at(x, buff);
    sprintf(buff, "     %s|%s                                 ",
            ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
    line_at(x, buff);
  } else if (x1 == 1) {
    line_at(x, "                                *      ");
    sprintf(buff, "             %s~%s=[,,_,,]:3               ", ANSI_COLOR_RED,
            ANSI_COLOR_RESET);
    line_at(x, buff);
    line_at(x, "                                       ");
    sprintf(buff, "     %s*%s                                 ",
            ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
    line_at(x, buff);
    line_at(x, "                                       ");
  } else if (x1 == 2) {
    line_at(x, "                                       ");
    sprintf(buff, "                                 %s.%s     ",
            ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
    line_at(x, buff);
    sprintf(buff, "             %s~%s=[,,_,,]:3               ", ANSI_COLOR_RED,
            ANSI_COLOR_RESET);
    line_at(x, buff);
    line_at(x, "     .                                 ");
    line_at(x, "                                       ");
  } else if (x1 == 3) {
    line_at(x, "                                       ");
    line_at(x, "                                       ");
    line_at(x, "                                       ");
    sprintf(buff, "             %s~%s=[,,_,,]:3               ", ANSI_COLOR_RED,
            ANSI_COLOR_RESET);
    line_at(x, buff);
    line_at(x, "                                       ");
  } else {
    line_at(x, "                                       ");
    line_at(x, "                                       ");
    sprintf(buff, "                                 %s*%s     ",
            ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
    line_at(x, buff);
    line_at(x, " .                                     ");
    line_at(x, "             ~=[,,_,,]:3               ");
  }

  usleep(FRAME_TIME * 8);
}

void clear_drawing(int x) {
  move_to_top();
  line_at(x, "                                       ");
  line_at(x, "                                       ");
  line_at(x, "                                       ");
  line_at(x, "                                       ");
  line_at(x, "                                       ");
  line_at(x, "                                       ");
  line_at(x, "                                       ");
}
