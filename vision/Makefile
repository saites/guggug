CFLAGS=`pkg-config --cflags --libs opencv`
CXXFLAGS=$(CFLAGS)

ALL=simple gray turkeybaster

all: $(ALL) 

clean:
	rm -f $(ALL) *.o

simple: simple.o
	gcc -o simple simple.o $(CFLAGS)

gray: gray.o
	gcc -o gray gray.o $(CFLAGS)

turkeybaster: turkeybaster.o disjoint.o
	gcc -o turkeybaster turkeybaster.o disjoint.o $(CFLAGS)
