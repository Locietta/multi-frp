#include "app.h"

#ifdef _WIN32

#include <windows.h>
#include <shellapi.h>

int wmain() {
    int argc;
    wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    App app;
    return app.run(argc, argv);
}

#else

int main(int argc, char *argv[]) {
    App app;
    return app.run(argc, argv);
}

#endif
