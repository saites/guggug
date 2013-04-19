#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>

#include <wiringPi.h>

long long counter = 0;
long long *mem;

void count() {
    mem[0]++;
}

int main(int argc, char* argv[]) {

    int shm_fd;

   /* open shared memory */
    shm_fd = shm_open("/encoder", O_CREAT | O_RDWR, 0666);

    if (ftruncate(shm_fd, 8)) {
	perror("Opening shared memory");
	exit(EXIT_FAILURE);
    }

    if ((mem = mmap(0, 8, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0)) == MAP_FAILED) {
	perror("Memory Map");
	exit(EXIT_FAILURE);
    }

    /* write 0 to the mem */
    mem[0] = 0;

    if (wiringPiSetup()) {
	perror("wiringPitSetup");
	exit(EXIT_FAILURE);
    }

    pinMode(7, INPUT);

    if (piHiPri(90)) {
	perror("piHiPri");
	exit(EXIT_FAILURE);
    }

    wiringPiISR(7, INT_EDGE_RISING, count);

    for (;;) {
	delay(1000);
	printf("Counter: %lli\n", mem[0]);
    }

    exit(EXIT_SUCCESS);
}
