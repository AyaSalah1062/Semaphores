CC = gcc
CFLAGS = -pthread

SOURCES = main.c
EXECUTABLE = program

all: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXECUTABLE)
