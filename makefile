magicGrader: main.c
	gcc -o magicgrader -std=gnu99 -Wall main.c
clean: magicgrader
	rm magicgrader
install:
	install -d $(DESTDIR)usr/bin/ && install magicgrader $(DESTDIR)usr/bin/
