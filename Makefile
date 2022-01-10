EXECUTABLE = hdr_metadata
OBJECTS = hdr_metadata.o

CFLAGS = -g -Wall -Wextra -pedantic -std=c99
LDFLAGS = -lm

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)

clean:
	-rm -f $(EXECUTABLE) $(OBJECTS)

.PHONY: all clean
