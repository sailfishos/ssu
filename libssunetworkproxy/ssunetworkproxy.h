/**
 * @file ssunetworkproxy.h
 * @copyright 2014 Jolla Ltd.
 * @author Juha Kallioinen <juha.kallioinen@jolla.com>
 * @date 2014
 */

#ifndef _LibSsuNetworkProxy_H
#define _LibSsuNetworkProxy_H

#include <dlfcn.h>

#include <QtCore/QtGlobal>
#include <QtNetwork/QNetworkProxyFactory>

/**
 * Set application proxy. First check \c http_proxy environment variable, then try to get
 * proxy configuration from connman.
 */
inline void set_application_proxy_factory()
{
    if (qEnvironmentVariableIsSet("http_proxy")) {
        qDebug() << "Got http_proxy from environment, will not talk to connman";
        QNetworkProxyFactory::setUseSystemConfiguration(true);
        return;
    }

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
