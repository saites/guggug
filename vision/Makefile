CFLAGS=`pkg-config --cflags --libs opencv`
CXXFLAGS=$(CFLAGS)

ALL=simple

all: $(ALL) 

clean:
	rm -f $(ALL) *.o

simple: simple.o
	gcc -o simple simple.o $(CFLAGS)


