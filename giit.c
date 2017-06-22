/*
 * giit - a git launcher
 * Insprired by gti and sl :D
 * Not support for MS Windows.
 *
 */

#define _POSIX_C_SOURCE 199309L
#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

static const char *GIT_CMD = "git";
static const char *GIT_CL = "clone";
static const char *GIT_PSH = "push";
static const char *GIT_PL = "pull";
static const char *GIT_STS = "status";
static const int GIIT_SPEED = 1000;

FILE *TERM_FH;
int TERM_WIDTH;
int FRAME_TIME;
static const int START_AT_Y = -20;
static const int PUSH_MOVEMENT[] = {0, 1, 2, 3, 4, 3, 2, 1};
static const int NUMBER_OF_LINE = 5;
static const int BUFFER_SIZE = 1024;

void get_term();
int get_term_width(void);
void init(void);
void clear_all(int x);
void clear_at(int x);
void move_to_top(void);
void draw(int x, char *s);
void draw_empty(int x);

void generate_std_art(int x);
void draw_std(int x);
void draw_clone(int x);
void draw_push(int x);
void draw_pull(int x);
void draw_status(int x);
typedef void (*draw_fn)(int x);
draw_fn resolve_draw_fn(int argc, char **argv);

// To keep your eyes unhurt :)!
char *lines[NUMBER_OF_LINE];

int main(int argc, char **argv)
{
  char *tmp;
  draw_fn draw_fn;
  int errorCode = 0;
  for (size_t i = 0; i < NUMBER_OF_LINE; ++i)
  {
    lines[i] = (char *)malloc(BUFFER_SIZE);
  }

  TERM_FH = fopen("/dev/tty", "w");
  if (!TERM_FH)
  {
    errorCode = 1;
  }
  if (!errorCode)
  {
    TERM_WIDTH = get_term_width();
    FRAME_TIME = 1000 * 1000 * 10 / (GIIT_SPEED + TERM_WIDTH + 1);

    draw_fn = resolve_draw_fn(argc, argv);
    init();
    for (int i = START_AT_Y; i < TERM_WIDTH; ++i)
    {
      draw_fn(i);
      clear_all(i);
    }
    move_to_top();
    fflush(TERM_FH);

    execvp(GIT_CMD, argv);
  }

  perror(GIT_CMD);

  for (size_t i = 0; i < NUMBER_OF_LINE; ++i)
  {
    free(lines[i]);
  }

  return 1;
}

int get_term_width(void)
{
  struct winsize w;
  ioctl(fileno(TERM_FH), TIOCGWINSZ, &w);
  return w.ws_col;
}

draw_fn resolve_draw_fn(int argc, char **argv)
{
  //TODO: add more art work!!!
  /*
  if (argc > 1)
  {
    if (!strcmp(argv[1], GIT_PSH))
    {
      return &draw_push;
    }

    if (!strcmp(argv[1], GIT_PL))
    {
      return &draw_pull;
    }
  }
  */

  return &draw_std;
}

void init(void)
{
  char buff[BUFFER_SIZE];
  strcpy(buff, "");
  for (size_t i = 0; i < NUMBER_OF_LINE; ++i)
  {
    strcat(buff, "\n");
  }
  fputs(buff, TERM_FH);
  fflush(TERM_FH);
}

void clear_all(int x)
{
  move_to_top();
  for (size_t i = 1; i < NUMBER_OF_LINE + 1; ++i)
  {
    clear_at(x);
  }
}

void clear_at(int x)
{
  draw_empty(x);
}

void draw_empty(int x)
{
  draw(x, "                                               ");
}

void move_to_top(void)
{
  fprintf(TERM_FH, "\033[7A");
}

void move_to_x(int x)
{
  fprintf(TERM_FH, "\033[%dC", x);
}

void draw(int x, char *s)
{
  int y;
  size_t i;

  if (x > 1)
  {
    move_to_x(x);
  }
  for (y = x, i = 0; i < strlen(s); y++, i++)
  {
    if (y > 0 && y < TERM_WIDTH)
      fputc(s[i], TERM_FH);
  }
  fputc('\n', TERM_FH);

  fflush(TERM_FH);
}

void draw_std(int x)
{
  generate_std_art(x);
  move_to_top();
  for (int i = 0; i < NUMBER_OF_LINE; ++i)
  {
    draw(x, lines[i]);
  }
  usleep(FRAME_TIME * 4);
}

void generate_std_art(int x)
{
  int x1 = PUSH_MOVEMENT[(x - START_AT_Y) % 8];
  if (x1 == 0)
  {
    strcpy(lines[0], "             ~=[,,_,,]:3               ");
    strcpy(lines[1], "                                       ");
    strcpy(lines[2], "     |                                 ");
    strcpy(lines[3], "   --*--                               ");
    strcpy(lines[4], "     |                                 ");
  }
  else if (x1 == 1)
  {
    strcpy(lines[0], "                                       ");
    strcpy(lines[1], "             ~=[,,_,,]:3               ");
    strcpy(lines[2], "                                       ");
    strcpy(lines[3], "     *                                 ");
    strcpy(lines[4], "                                       ");
  }
  else if (x1 == 2)
  {
    strcpy(lines[0], "                                  *    ");
    strcpy(lines[1], "                                       ");
    strcpy(lines[2], "             ~=[,,_,,]:3               ");
    strcpy(lines[3], "     .                                 ");
    strcpy(lines[4], "                                       ");
  }
  else if (x1 == 3)
  {
    strcpy(lines[0], "                                   .   ");
    strcpy(lines[1], "                                       ");
    strcpy(lines[2], "                                       ");
    strcpy(lines[3], "             ~=[,,_,,]:3               ");
    strcpy(lines[4], "                                       ");
  }
  else
  {
    strcpy(lines[0], "                                   +   ");
    strcpy(lines[1], "                                       ");
    strcpy(lines[2], "                                       ");
    strcpy(lines[3], "                                       ");
    strcpy(lines[4], "             ~=[,,_,,]:3               ");
  }
}
