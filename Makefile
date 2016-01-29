CC = clang
CFLAGS = -g -Wall -Wextra -march=native -DNDEBUG $(OPTFLAGS)
PREFIX?=$(HOME)

SOURCES=$(wildcard src/*.c)
OBJECTS=$(patsubst src/%.c,build/%.o,$(SOURCES))

TARGET=build/todo

.PHONY: all
all: build $(TARGET)

.PHONY: clean
clean:
	rm -r build

build:
	mkdir -p build

$(TARGET): $(OBJECTS)

build/%.o: src/%.c
	$(CC) -o $@ -c $(CFLAGS) $<

.PHONY: run
run: all
	@./$(TARGET)

.PHONY: install
install: all
	install -d $(DESTDIR)/$(PREFIX)/bin
	install $(TARGET) $(DESTDIR)/$(PREFIX)/bin
