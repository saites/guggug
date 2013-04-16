CFLAGS=-DMODELDIR="`pkg-config --variable=modeldir pocketsphinx`" \
		`pkg-config --cflags --libs pocketsphinx sphinxbase`
ALL=recognize moveforward

all: $(ALL) 

clean:
	rm -f $(ALL) *.o

moveforward: moveforward.o
	gcc -o moveforward moveforward.o $(CFLAGS)

recognize: recognize.o
	gcc -o recognize recognize.o $(CFLAGS)
