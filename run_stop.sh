#!/bin/bash

APP1="./build/app1"
APP2="./build/app2"

start_apps() {
    if [ ! -f "$APP1" ]; then
        echo "Error: $APP1 not found!"
        exit 1
    fi

    if [ ! -f "$APP2" ]; then
        echo "Error: $APP2 not found!"
        exit 1
    fi        
    gnome-terminal --tab --title="App1" -- bash -c "$APP1; exit"
    gnome-terminal --tab --title="App2" -- bash -c "$APP2; exit"    
}

stop_apps() {
    APP1_NAME=$(basename "$APP1")
    APP2_NAME=$(basename "$APP2")
    pkill -x "$APP1_NAME"
    pkill -x "$APP2_NAME"
}

case "$1" in
    start)
        start_apps
        ;;
    stop)
        stop_apps
        ;;
    *)
        echo "Usage: $0 {start|stop}"
        exit 1
        ;;
esac
