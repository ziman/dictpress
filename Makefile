dictpress: dictpress.c bitio.h bitio.c fgk.c fgk.h
	cc dictpress.c bitio.c fgk.c -O2 -o dictpress -W -Wall

clean:
	rm -f *.o *~ dictpress
