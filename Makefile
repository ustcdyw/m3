magic3: magic3.c
	cc -o magic3 magic3.c -lcurses -DSTATISTICS

clean:
	rm -f magic3
