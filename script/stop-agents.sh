#!/bin/bash


PIDS=$(ps -eo pid,command | grep "MonitoringAgent" | grep -v grep | awk '{print $1}')

kill $PIDS
