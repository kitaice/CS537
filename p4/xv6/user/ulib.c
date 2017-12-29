#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

char*
strcpy(char *s, char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, void *vsrc, int n)
{
  char *dst, *src;
  
  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
}

void
lock_init(lock_t *lock)
{
  lock->flag = 0;
}

void
lock_acquire(lock_t *lock)
{
  while(xchg(&lock->flag,1)==1);
}

void
lock_release(lock_t *lock)
{
  xchg(&lock->flag,0);
}
void
cond_init(cond_t * cv)
{
   cv->lock = 0;
}
void
cond_wait(cond_t *cv, lock_t *lock)
{ 
  tsleep(cv, lock);
  return;
}
void
cond_signal(cond_t *cv)
{
  twake(cv);
  return;
}

void
sem_init(sem_t *sem, int init)
{
  sem->value = init;
  lock_init(&sem->lock);
  cond_init(&sem->cv);
}

void
sem_wait(sem_t * sem)
{
  lock_acquire(&sem->lock);
  sem->value --;
  if(sem->value < 0)
   tsleep(&sem->cv, &sem->lock);
  lock_release(&sem->lock);
}

void
sem_post(sem_t * sem)
{
  lock_acquire(&sem->lock);
  sem->value ++;
  twake(&sem->cv);
  lock_release(&sem->lock);
}
