
#include <stdlib.h>
#include <stdarg.h>
#include "Logger.h"

void Logger::addLog(const wchar_t* fmt, ...)
{
    wchar_t cText[ 1024 ];
    va_list ap;

    va_start(ap, fmt);
    vswprintf(cText, 1024, fmt, ap);
    va_end(ap);

    FILE *f = fopen("logger.log", "aw");
    fwprintf(f, L"\n%ls", cText);
    fclose(f);
}
