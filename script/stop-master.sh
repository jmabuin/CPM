#!/bin/bash


PIDS=$(ps -eo pid,command | grep "MonitoringMaster" | grep -v grep | awk '{print $1}')

kill $PIDS
