#!/bin/bash

eosiocpp -o testtokeneos.wast testtokeneos.cpp
eosiocpp -g testtokeneos.abi testtokeneos.cpp
cleos set contract token ../testtokeneos -p token@active
