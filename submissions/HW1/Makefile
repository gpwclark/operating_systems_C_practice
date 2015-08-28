## Copyright (c) 2013, UNC Chapel Hill. All rights reserved.
## Author: Andrew M. White
##
## Simple makefile for introductory COMP535 lab programs.

# macros to define commonly used strings
CC = g++
CFLAGS = -Wall -Wstrict-null-sentinel -Wextra -Wwrite-strings -fno-common -pedantic -g2 

# general Makefile syntax
# <target name> : <target prerequisites>
#       <command(s) to build target>
#
# target name:    generally, a file to check for updated prereqs and build
#                                                                       if necessary
# prerequisites:  files or other targets required to build current target
# commands:                     shell commands to build target given prereqs

# special variables
# '$@': target name
# '$?': target's prereqs

warmup: warmup.c
	$(CC) $(CFLAGS) -o $@ $?
