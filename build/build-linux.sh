#!/bin/bash -x

cc search.c persist.c main.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o ./bin/main-linux
