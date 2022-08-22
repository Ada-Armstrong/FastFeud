#!/bin/sh

exectuable="build/FastFeud"
log="config/last_session.log"

if ! [ -f "$exectuable" ]; then
	echo "Exectuable missing, build the project..."
	exit 1
fi

if ! [ -f "$log" ]; then
	touch "$log"
fi

$exectuable 2>&1 | tee -i "$log"
