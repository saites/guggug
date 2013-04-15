all: moveforward

clean:
	rm -f moveforward

moveforward:
	gcc -o moveforward moveforward.c \
		-DMODELDIR=\"`pkg-config --variable=modeldir pocketsphinx`\" \
		`pkg-config --cflags --libs pocketsphinx sphinxbase`
