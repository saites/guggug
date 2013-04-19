#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>

#include <wiringPi.h>

#define SHM_SIZE    16
#define SHM_NAME    "/encoder"

long long *mem;
int shm_fd;

void signal_callback_handler(int);

void lcount() {
    mem[0]++;
}

void rcount() {
    mem[1]++;
}

int main(int argc, char* argv[]) {

    /* register our signal handler */
    signal(SIGINT, signal_callback_handler);

   /* open shared memory */
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
	perror("Opening shared memory file");
	exit(EXIT_FAILURE);
    }

    if ((mem = mmap(0, SHM_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0)) == MAP_FAILED) {
	perror("Mapping memory");
	exit(EXIT_FAILURE);
    }

    /* write 0 to the mem */
    mem[0] = 0;
    mem[1] = 0;

    if (wiringPiSetup() != 0) {
	perror("wiringPitSetup");
	exit(EXIT_FAILURE);
    }

    pinMode(7, INPUT);
    pinMode(0, INPUT);

    if (piHiPri(90) != 0) {
	perror("piHiPri");
	exit(EXIT_FAILURE);
    }

    wiringPiISR(7, INT_EDGE_RISING, lcount);
    wiringPiISR(0, INT_EDGE_RISING, rcount);

    for (;;) {
	delay(1000);
	printf("Counter:\t %lli \t %lli\n", mem[0], mem[1]);
    }

    exit(EXIT_SUCCESS);
}


void signal_callback_handler(int signum) {

    /* cleanup */
    if (munmap(mem, SHM_SIZE) == -1) {
	perror("Error un-mmapping the file");
	close(shm_fd);
	exit(EXIT_FAILURE);
    }

    close(shm_fd);

    if (shm_unlink(SHM_NAME) != 0) {
	perror("Error unlinking");
	exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);

}
