#ifndef MAIN_H
#define MAIN_H

#define PDF_PRESENTER_VERSION "0.1"

#ifdef WIN32
    #define realpath(N,R) _fullpath((R),(N),PATH_MAX)
#endif

extern GtkApplication* app;

#endif // MAIN_H
