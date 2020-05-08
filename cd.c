#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf(1, "Error, need more arguments.\n");
    exit();
  }
  if (chdir(argv[1]) < 0)
  {
    printf(1, "Error, cannot change the directory\n");
  }
  exit();
}