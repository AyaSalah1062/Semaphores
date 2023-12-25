CC = gcc
CFLAGS = -pthread

SOURCES = 7361.c
EXECUTABLE = program

all: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXECUTABLE)
