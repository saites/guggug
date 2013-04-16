CFLAGS=-DMODELDIR="`pkg-config --variable=modeldir pocketsphinx`" \
		`pkg-config --cflags --libs pocketsphinx sphinxbase`

all: moveforward 

clean:
	rm -f moveforward moveforward.o

moveforward: moveforward.o
	gcc -o moveforward moveforward.o $(CFLAGS)
