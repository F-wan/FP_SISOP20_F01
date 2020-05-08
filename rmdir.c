#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

char *fmtname(char *path)
{
  static char buf[DIRSIZ + 1];
  char *p;

  // Find first character after last slash.
  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if (strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
  return buf;
}

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    printf(2, "Usage: rmdir directory...\n");
    exit();
  }

  for (int i = 1; i < argc; i++)
  {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    int counter = 0;

    if ((fd = open(argv[i], 0)) < 0)
    {
      printf(2, "rmdir: cannot open %s\n", argv[i]);
      exit();
    }

    if (fstat(fd, &st) < 0)
    {
      printf(2, "rmdir: cannot stat %s\n", argv[i]);
      close(fd);
      exit();
    }
    
    switch (st.type)
    {
      case T_FILE:
        printf(2, "rmdir: %s is not directory\n", argv[i]);
        close(fd);
        exit();

      case T_DIR:
        if (strlen(argv[i]) + 1 + DIRSIZ + 1 > sizeof buf)
        {
          printf(1, "rmdir: path too long\n");
          break;
        }

        strcpy(buf, argv[i]);
        p = buf + strlen(buf);
        *p++ = '/';

        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
          if (de.inum == 0) continue;

          memmove(p, de.name, DIRSIZ);
          p[DIRSIZ] = 0;
          if (stat(buf, &st) < 0)
          {
            printf(1, "rmdir: cannot stat %s\n", buf);
            continue;
          }
          counter++;

          switch (st.type)
          {
          case T_FILE:
            printf(1, "FILE\t: %s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
            break;
          
          case T_DIR:
            printf(1, "DIR\t: %s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
            break;
          }
        }
        break;
    }


    if (counter > 2)
    {
      printf(1, "counter = %d\n", counter);
      printf(2, "rmdir: failed to remove '%s': Directory not empty\n", argv[i]);
    }
    else
    {
      unlink(argv[i]);
      printf(1, "rmdir: %s removed\n", argv[i]);
    }
  }
  exit();
}
