#!/bin/sh

export LD_LIBRARY_PATH="`dirname "$0"`:${LD_LIBRARY_PATH}"
exec "`dirname "$0"`/$1"
