DEPS = dictpress.c bitio.h bitio.c fgk.c fgk.h 

dictpress: $(DEPS)
	cc $(DEPS) -O2 -o dictpress -W -Wall

debug:
	cc $(DEPS) -O0 -o dictpress -W -Wall -g3 -ggdb -DDEBUG

clean:
	rm -f *.o *~ dictpress
