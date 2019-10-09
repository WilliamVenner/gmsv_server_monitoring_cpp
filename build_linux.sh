#!/bin/bash
g++ -m32 -fPIC -shared -I include server_monitoring/src/gm_server_monitoring.cpp -o server_monitoring/build/gmsv_server_monitoring_linux.dll