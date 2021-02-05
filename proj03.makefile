# Hanshi Zuo
# makefile for project 3

exe = proj03

objs = $(exe).student.o
file = $(exe).student.cpp

CC = g++
CFLAGS = -std=c++11 -Wall -Wextra -O2 -g2

.PHONY: all

all: $(exe)

$(exe): $(file)
	$(CC) $(CFLAGS) -o $(exe) $(file)

.PHONY: clean
clean:
	rm -f $(exe)
	rm -f $(objs)

