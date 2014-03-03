/**
 * @file ssunetworkproxy.h
 * @copyright 2014 Jolla Ltd.
 * @author Juha Kallioinen <juha.kallioinen@jolla.com>
 * @date 2014
 */

#ifndef _LibSsuNetworkProxy_H
#define _LibSsuNetworkProxy_H
#include <dlfcn.h>

/**
 * Set application proxy if the required library is found, otherwise
 * do nothing.
 */
inline void set_application_proxy_factory()
{
    void *proxylib = dlopen("libssunetworkproxy.so", RTLD_LAZY);
    if (proxylib) {
        typedef void (*ssuproxyinit_t)();
        dlerror();
        ssuproxyinit_t proxy_init = (ssuproxyinit_t) dlsym(proxylib, "initialize");
        const char *dlsym_err = dlerror();
        if (!dlsym_err) {
            proxy_init();
        }
        dlclose(proxylib);
    }
}

#endif
