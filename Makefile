dictpress: dictpress.c
	cc dictpress.c -O2 -o dictpress

clean:
	rm -f *.o *~ dictpress
