.PHONY: all clean

TARGET = ./dph
CC = gcc
CFLAGS = -Wall -Wextra -g -O0
LDFLAGS = -lm
OBJS = rs.o fp.o globals.o rule.o histogram.o dp2d.o

all: $(TARGET)

$(TARGET): $(OBJS)

clean:
	@$(RM) $(OBJS) $(TARGET)
