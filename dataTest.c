#include "data.h"
#include <stdio.h>

int main(int argc, char **argv) {
  gra_db_t *db;
  GError *err=NULL;

  db = gra_db_open(argv[1], &err);
  gra_db_close(db, &err);

  if(err) {
    printf("%s\n", err->message);
  }

  return 0;
}
