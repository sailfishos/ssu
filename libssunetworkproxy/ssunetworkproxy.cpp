/**
 * @file ssunetworkproxy.cpp
 * @copyright 2014 Jolla Ltd.
 * @author Juha Kallioinen <juha.kallioinen@jolla.com>
 * @date 2014
 */

#include <connman-qt5/connmannetworkproxyfactory.h>

extern "C" void initialize()
{
    QNetworkProxyFactory::setApplicationProxyFactory(new ConnmanNetworkProxyFactory);
}
