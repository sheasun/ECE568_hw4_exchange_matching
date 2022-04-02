#!/bin/bash
#sudo su - postgres & psql & CREATE DATABASE stock_db;
make clean
make all
echo 'start to running server...'
./server &
while true ; do continue ; done
