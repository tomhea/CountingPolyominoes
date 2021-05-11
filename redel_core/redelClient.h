#ifndef COUNTINGPOLYOMINOES_REDELCLIENT_H
#define COUNTINGPOLYOMINOES_REDELCLIENT_H

#include "defs.h"

extern "C" {
u64 executeJob(const char *graphFilePath, const char *jobPath);
}

#endif //COUNTINGPOLYOMINOES_REDELCLIENT_H
