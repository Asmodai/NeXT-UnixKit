#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libc.h>

extern char **environ;

static char *__progname = NULL;
static int   allocated  = 0;

static
void
usage(void)
{
  fprintf(stderr,
          "usage: env [-i] [name=value ...] [utility [argument ...]]\n");
  exit(EXIT_FAILURE);
}

static
int
namelength(char *name)
{
  char *equal = NULL;

  equal = strchr(name, '=');
  return ((equal == 0) ? strlen(name) : (equal - name));
}

static
char **
findenv(char *name, int len)
{
  char **envp = NULL;

  for (envp = environ; envp && *envp; envp++) {
    if (strncmp(name, *envp, len) == 0 && (*envp)[len] == '=') {
      return envp;
    }
  }

  return NULL;
}

static
char *
cmalloc(int new_len, char *old, int old_len)
{
  char *new = malloc(new_len);

  if (new != NULL) {
    memcpy(new, old, old_len);
  }

  return new;
}

static
int
addenv(char *nameval)
{
  char **envp   = NULL;
  int    n_used = 0;
  int    l_used = 0;
  int    l_need = 0;

  for (envp = environ; envp && *envp; envp++)
    ;

  n_used = envp - environ;
  l_used = n_used * sizeof(*envp);
  l_need = l_used + 2 * sizeof(*envp);

  envp = allocated
    ? (char **)realloc((char *)environ, l_need)
    : (char **)cmalloc(l_need, (char *)environ, l_used);

  if (envp == NULL) {
    return -1;
  } else {
    allocated = 1;
    environ = envp;
    environ[n_used++] = nameval;
    environ[n_used]   = 0;
    
    return 0;
  }
}

static
int
setenv(char *name, char *value, int clobber)
{
  char  *destination = NULL;
  char **envp        = NULL;
  int    l_name      = 0;
  int    l_nameval   = 0;

  l_name = namelength(name);
  envp   = findenv(name, l_name);

  if (envp != NULL && clobber == 0) {
    return 0;
  }

  if (*value == '=') {
    value++;
  }

  l_nameval   = l_name + strlen(value) + 1;
  destination = (envp != 0 && strlen(*envp) >= l_nameval)
    ? *envp
    : malloc(l_nameval + 1);
  if (destination == NULL) {
    return -1;
  }

  strncpy(destination, name, l_name);
  destination[l_name] = '=';
  strcpy(destination + l_name + 1, value);

  return ((envp == 0)
          ? addenv(destination)
          : (*envp = destination, 0));
}


static
int
putenv(char *nameval)
{
  char *equal = strchr(nameval, '=');
  char *value = (equal ? equal : "");

  return (setenv(nameval, value, 1));
}

static
void
putcolsp(void)
{
  fputc(':', stderr);
  fputc(' ', stderr);
}

static
void
putprog(void)
{
  fputs(__progname, stderr);
  putcolsp();
}

static
void
verr(int eval, const char *fmt, va_list ap)
{
  int sverrno = 0;

  sverrno = errno;
  putprog();
  
  if (fmt != NULL) {
    vfprintf(stderr, fmt, ap);
    putcolsp();
  }

  fputs(strerror(sverrno), stderr);
  fputc('\n', stderr);

  exit(eval);
}

static
void
err(int eval, const char *fmt, ...)
{
  va_list ap = NULL;

  va_start(ap, fmt);
  verr(eval, fmt, ap);
  va_end(ap);
}

int
main(int argc, char **argv)
{
  char **ep          = NULL;
  char  *p           = NULL;
  char  *cleanenv[1] = { NULL };
  int    ch          = 0;

  __progname = argv[0];

  while ((ch = getopt(argc, argv, "-i")) != -1) {
    switch (ch) {
      case '-':
      case 'i':
        environ     = cleanenv;
        cleanenv[0] = NULL;
        break;

      case '?':
      default:  
        usage();
    }
  }

  for (argv += optind; *argv && (p = strchr(*argv, '=')); ++argv) {
    putenv(*argv);
  }

  if (*argv) {
    execvp(*argv, argv);
    err(errno == ENOENT ? 127 : 126, "%s", *argv);
  }

  for (ep = environ; *ep; ep++) {
    printf("%s\n", *ep);
  }

  return 0;
}
