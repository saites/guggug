#include <stdio.h>
#include <stdlib.h>
#include <pocketsphinx.h>

#define EXIT_ERROR fprintf(stderr, "error processing %s on line %d\n",\
		__FILE__, __LINE__);\
	exit(1);
#define MODELDIR "/home/saites/sphinx/pocketsphinx-0.8/model"

int
main(int argc, char *argv[]) {
	ps_decoder_t *ps;
	cmd_ln_t *config;
	FILE *fh;
	const char *filename = "goforward.raw";
	const char *word = "goforward";
	char const *hyp, *uttid;
	int rv;
	int32 score;

	/* setup the sphinx config */
	config = cmd_ln_init(NULL, ps_args(), TRUE,
			"-hmm", MODELDIR "/hmm/en_US/hub4wsj_sc_8k", 
			"-lm", MODELDIR "/lm/en/turtle.DMP",
			"-dict", MODELDIR "/lm/en/turtle.dic",
			NULL);
	if(config == NULL) {
		EXIT_ERROR;
	}

	/* initialize the config */
	ps = ps_init(config);
	if(ps == NULL) {
		EXIT_ERROR;
	}

	/* open the audio file (stream?) */
	fh = fopen(filename, "rb");
	if(fh == NULL) {
		perror(filename);
		exit(1);
	}

	/* decode the file */
	rv = ps_decode_raw(ps, fh, word, -1);
	if(rv < 0) {
		EXIT_ERROR;
	}

	/* get hypothesis */
	hyp = ps_get_hyp(ps, &score, &uttid);
	if(hyp == NULL) {
		EXIT_ERROR;
	}
	printf("Recognized: %s\n", hyp);

	/* clean up */
	fclose(fh);
	ps_free(ps);

	return 0;
}
