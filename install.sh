#!/bin/bash

init_process=$(ps --no-headers -o comm 1)

if [ "$init_process" = "systemd" ]; then
    # systemd-based init system
    echo "========================================"
    echo "Identified systemd based OS"
    echo "========================================"
    install Qme_systemd /etc/systemd/system/Qme.service
    systemctl daemon-reload
    systemctl start Qme.service
    systemctl enable Qme.service
    systemctl status Qme.service
    echo ""
    echo "========================================"
    :
elif [ "$init_process" = "init" ]; then
    # SysV init system
    echo "========================================"
    echo "Identified SysV init based OS"
    echo "========================================"
    install Qme_init /etc/init.d/Qme
    /etc/init.d/Qme start
    rc-update add Qme
    echo ""
    echo "========================================"
    :
else
    echo "No valid init system found (got: $init_process)"
    :
fi

echo "FINISHED setting up init!"
