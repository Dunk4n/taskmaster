#include "taskmaster.h"

void err_display(const char *msg, const char *file, const char *func,
		 uint32_t line) {
#ifdef DEVELOPEMENT
  fprintf(stderr,
	  "" BRED "ERROR" CRESET ": in file " BWHT "%s" CRESET
	  " in function " BWHT "%s" CRESET " at line " BWHT "%d" CRESET
	  "\n    %s\n",
	  file, func, line, msg);
#endif

#ifdef DEMO
  fprintf(stderr,
	  "" BRED "ERROR" CRESET ": in file " BWHT "%s" CRESET " at line " BWHT
	  "%s" CRESET "\n",
	  file, line);
#endif

#ifdef PRODUCTION
  fprintf(stderr, "" BRED "ERROR" CRESET "\n");
#endif
}
