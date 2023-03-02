CC = gcc
FLAGS = -Wall -Wextra -Werror -std=c11
SOURCES = decimal.c

all: 
	$(CC) $(FLAGS) $(SOURCES) -o decimal
	