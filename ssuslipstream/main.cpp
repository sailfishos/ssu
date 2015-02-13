/**
 * ssu: Seamless Software Update
 * Copyright (C) 2015 Jolla Ltd.
 * Contact: Thomas Perl <thomas.perl@jolla.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **/

#include <QCoreApplication>
#include <QTimer>

#include "libssunetworkproxy/ssunetworkproxy.h"

#include "ssuslipstream.h"


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    set_application_proxy_factory();

    SsuSlipstream slipstream;
    QTimer::singleShot(0, &slipstream, SLOT(run()));

    return app.exec();
}
