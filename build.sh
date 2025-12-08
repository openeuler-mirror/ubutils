#!/bin/bash

if [ $# -eq 0 ]; then
    echo "No param! Please add install or uninstall to execute."
    exit 1
fi

param=$1
DATE=2025-10-28

case $param in
    "install")
        echo "--- Installing ---"
        mkdir build
        cd build
        cmake ..
        make
        install -c -m 755 -s lsub/lsub /usr/bin
        install -c -m 755 -s setub/setub /usr/bin
        cd ..
        gzip -9n <ub.ids >ub.ids.gz
        M=`echo $DATE | sed 's/-01-/-January-/;s/-02-/-February-/;s/-03-/-March-/;s/-04-/-April-/;s/-05-/-May-/;s/-06-/-June-/;s/-07-/-July-/;s/-08-/-August-/;s/-09-/-September-/;s/-10-/-October-/;s/-11-/-November-/;s/-12-/-December-/;s/\(.*\)-\(.*\)-\(.*\)/\3 \2 \1/'` ; sed <setub.man >setub.8 "s/@TODAY@/$M/;s/@VERSION@/ubutils-1.0.1/;s#@IDSDIR@#/usr/share/hwdata#"
        M=`echo $DATE | sed 's/-01-/-January-/;s/-02-/-February-/;s/-03-/-March-/;s/-04-/-April-/;s/-05-/-May-/;s/-06-/-June-/;s/-07-/-July-/;s/-08-/-August-/;s/-09-/-September-/;s/-10-/-October-/;s/-11-/-November-/;s/-12-/-December-/;s/\(.*\)-\(.*\)-\(.*\)/\3 \2 \1/'` ; sed <lsub.man >lsub.8 "s/@TODAY@/$M/;s/@VERSION@/ubutils-1.0.1/;s#@IDSDIR@#/usr/share/hwdata#"
        install -d -m 755 /usr/bin /usr/share/hwdata /usr/share/man/man8 /usr/share/man/man5
        install -c -m 644 ub.ids.gz /usr/share/hwdata
        install -c -m 644 lsub.8 setub.8 /usr/share/man/man8
        rm lsub.8 setub.8 ub.ids.gz
        echo "--- Installing complete ---"
        ;;
    "uninstall")
        echo "--- Uninstalling ---"
        rm /usr/share/man/man8/lsub.8
        rm /usr/share/man/man8/setub.8
        rm /usr/share/hwdata/ub.ids.gz
        rm /usr/bin/lsub
        rm /usr/bin/setub
        rm -rf build
        echo "--- Uninstalling complete ---"
        ;;
    *)
        echo "Unknown param: $param, use install or uninstall to execute."
        exit 1
        ;;
esac
