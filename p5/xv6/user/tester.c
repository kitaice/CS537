// Do not modify this file. It will be replaced by the grading scripts
// when checking your project.

#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "%s", "** Placeholder program for grading scripts **\n");
  struct stat st;
  stat(argv[1], &st);
  printf(1, "type=%d dev=%d ino=%d nl=%d sz=%d\n",
      st.type, st.dev, st.ino, st.nlink, st.size);
  for (int i = 0; i < NDIRECT+1; i++) {
    printf(1, "[%d] %x\n", i, st.addrs[i]);
  }
  char buf[BSIZE];
  block(buf, st.dev, st.addrs[0]);
  write(1, buf, 100);
  printf(1, "%s", "** Placeholder program for grading scripts **\n");
  exit();
}
