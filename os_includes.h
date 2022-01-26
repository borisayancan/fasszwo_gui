#ifndef OS_INCLUDES_H
#define OS_INCLUDES_H
#include <QtGlobal>
#include <math.h>


#ifdef Q_OS_WIN
#include "Windows.h"
#define sleep_ms(A)     Sleep(A)
#endif

#ifdef Q_OS_LINUX
#include <unistd.h>
#define sleep_ms(A)     usleep(A*1000)
#endif


#endif // OS_INCLUDES_H
