default: build

build:	src/monbacklight.c
	gcc -Wall -o monbacklight src/monbacklight.c

install: monbacklight
	install -m 6755 -D monbacklight $(DESTDIR)/usr/bin/monbacklight

remove:
	rm -fv $(DESTDIR)/usr/bin/monbacklight
