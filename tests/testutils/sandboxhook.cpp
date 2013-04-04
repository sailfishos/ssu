#include <dlfcn.h>

#include "libssu/sandbox_p.h"

extern "C" void qt_startup_hook()
{
   Sandbox *const sandbox = new Sandbox();
   Q_UNUSED(sandbox);

   static void(*next_qt_startup_hook)() = (void (*)()) dlsym(RTLD_NEXT, "qt_startup_hook");
   next_qt_startup_hook();
}
