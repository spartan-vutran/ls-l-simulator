#ifndef FILE_PERMS_H
#define FILE_PERMS_H

#include <sys/types.h>

char *filePermStr(mode_t perm, int flags);

#endif
