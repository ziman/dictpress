dictpress: dictpress.c bitio.h bitio.c
	cc dictpress.c bitio.c -O2 -o dictpress

clean:
	rm -f *.o *~ dictpress
