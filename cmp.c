#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int isSameFiles(int file1, int file2)
{
  char buf1[1024];
  char buf2[1024];
  int sz1 = read(file1, buf1, sizeof(buf1));
  int sz2 = read(file2, buf2, sizeof(buf2));
  
  buf1[sz1] = '\0';
  buf2[sz2] = '\0';
  
  int col = 0, line = 1, karakter = 0;
  int idx1 = 0, idx2 = 0;
  
  while (idx1 < sz1-1 && idx2 < sz2-1)
  {
    col++;
    karakter++;
    if (buf1[idx1] == '\n' && buf2[idx2] == '\n')
    {
      line++;
      col = 0;
    }
    if (buf1[idx1] != buf2[idx2])
    {
      printf(1,"Line Number : %d \tError Position : %d \tbyte ke-%d\n", line, col, karakter);
      return 0;
    }
    idx1++;
    idx2++;
  }
  return 1;
}

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    printf(2, "Usage: cmp 2 files...\n");
    exit();
  }

  int file1 = open(argv[1], 0);
  int file2 = open(argv[2], 0);
  struct stat st1;
  struct stat st2;

  if (file1 < 0)
  {
    printf(2,"Error : File %s cannot open", argv[1]);
    exit();
  }
  if (file2 < 0)
  {
    printf(2,"Error : File %s cannot open", argv[2]);
    exit();
  }
  
  /*stat*/
  if (fstat(file1, &st1) < 0)
  {
    printf(2, "Error : cannot stat %s\n", argv[1]);
    close(file1);
    exit();
  }
  if (fstat(file2, &st2) < 0)
  {
    printf(2, "Error : cannot stat %s\n", argv[2]);
    close(file2);
    exit();
  }

  // CEK TIPE
  if (st1.type != T_FILE)
  {
    printf(2, "cmp: %s is not a file\n", argv[1]);
    close(file1);
    exit();
  }
  if (st2.type != T_FILE)
  {
    printf(2, "cmp: %s is not a file\n", argv[2]);
    close(file2);
    exit();
  }
  

  /*COMPARE 2 FILE*/
  if(!isSameFiles(file1, file2))
  {
    printf(1,"\nFile not same\n");
  }
  else
  {
    printf(1,"\nFile is same\n");
  }
  
  close(file1);
  close(file2);
  exit();
}