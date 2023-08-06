#!/bin/bash -xe

cc main.c `pkg-config --libs --cflags raylib` -o ./bin/main-macos

