all: clean debug

debug: main.c
	gcc main.c -o dist/main.out -g -DDEBUG

run: debug
	./dist/main.out < $(FILE)

release: main.c
	gcc main.c -o dist/main

clean:
	rm -rf dist/
	mkdir -p dist/

help:
	@echo "targets: all debug release clean help"

.PHONY: all
