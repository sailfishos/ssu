#include <dlfcn.h>

#include "sandboxfileenginehandler.h"

extern "C" void qt_startup_hook()
{
   SandboxFileEngineHandler *const handler = new SandboxFileEngineHandler();
   Q_UNUSED(handler);

   static void(*next_qt_startup_hook)() = (void (*)()) dlsym(RTLD_NEXT, "qt_startup_hook");
   next_qt_startup_hook();
}
