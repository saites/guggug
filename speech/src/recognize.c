/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 1999-2010 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced
 * Research Projects Agency and the National Science Foundation of the
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
#include <stdio.h>
#include <string.h>

#if !defined(_WIN32_WCE)
#include <signal.h>
#include <setjmp.h>
#endif
#if defined(WIN32) && !defined(GNUWINCE)
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <sphinxbase/err.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/cont_ad.h>

#include "pocketsphinx.h"

static const arg_t cont_args_def[] = {
    POCKETSPHINX_OPTIONS,
    /* Argument file. */
    { "-argfile",
      ARG_STRING,
      NULL,
      "Argument file giving extra arguments." },
    { "-adcdev",
      ARG_STRING,
      NULL,
      "Name of audio device to use for input." },
    { "-infile",
      ARG_STRING,
      NULL,
      "Audio file to transcribe." },
    { "-time",
      ARG_BOOLEAN,
      "no",
      "Print word times in file transcription." },
    CMDLN_EMPTY_OPTION
};

static ps_decoder_t *ps;
static cmd_ln_t *config;
static FILE* rawfd;

static int32
ad_file_read(ad_rec_t * ad, int16 * buf, int32 max)
{
    size_t nread;

    nread = fread(buf, sizeof(int16), max, rawfd);

    return (nread > 0 ? nread : -1);
}

static void
print_word_times(int32 start)
{
	ps_seg_t *iter = ps_seg_iter(ps, NULL);
	while (iter != NULL) {
		int32 sf, ef, pprob;
		float conf;

		ps_seg_frames (iter, &sf, &ef);
		pprob = ps_seg_prob (iter, NULL, NULL, NULL);
		conf = logmath_exp(ps_get_logmath(ps), pprob);
		fprintf (stderr, "%s %f %f %f\n", ps_seg_word (iter), (sf + start) / 100.0, (ef + start) / 100.0, conf);
		iter = ps_seg_next (iter);
	}
}

/*
 * Continuous recognition from a file
 */
static void
recognize_from_file() {
    cont_ad_t *cont;
    ad_rec_t file_ad = {0};
    int16 adbuf[4096];
    const char* hyp;
    const char* uttid;
    int32 k, ts, start;

    char waveheader[44];
    if ((rawfd = fopen(cmd_ln_str_r(config, "-infile"), "rb")) == NULL) {
	E_FATAL_SYSTEM("Failed to open file '%s' for reading",
			cmd_ln_str_r(config, "-infile"));
    }

    fread(waveheader, 1, 44, rawfd);

    file_ad.sps = (int32)cmd_ln_float32_r(config, "-samprate");
    file_ad.bps = sizeof(int16);

    if ((cont = cont_ad_init(&file_ad, ad_file_read)) == NULL) {
        E_FATAL("Failed to initialize voice activity detection");
    }
    if (cont_ad_calib(cont) < 0)
        E_FATAL("Failed to calibrate voice activity detection\n");
    rewind (rawfd);

    for (;;) {

	while ((k = cont_ad_read(cont, adbuf, 4096)) == 0);

        if (k < 0) {
    	    break;
    	}

        if (ps_start_utt(ps, NULL) < 0)
            E_FATAL("ps_start_utt() failed\n");

        ps_process_raw(ps, adbuf, k, FALSE, FALSE);

        ts = cont->read_ts;
        start = ((ts - k) * 100.0) / file_ad.sps;

        for (;;) {
            if ((k = cont_ad_read(cont, adbuf, 4096)) < 0)
            	break;

            if (k == 0) {
                /*
                 * No speech data available; check current timestamp with most recent
                 * speech to see if more than 1 sec elapsed.  If so, end of utterance.
                 */
                if ((cont->read_ts - ts) > DEFAULT_SAMPLES_PER_SEC)
                    break;
            }
            else {
                /* New speech data received; note current timestamp */
                ts = cont->read_ts;
            }


            ps_process_raw(ps, adbuf, k, FALSE, FALSE);
        }

        ps_end_utt(ps);

        if (cmd_ln_boolean_r(config, "-time")) {
	    print_word_times(start);
	} else {
	    hyp = ps_get_hyp(ps, NULL, &uttid);
            fprintf(stderr, "%s: %s\n", uttid, hyp);
        }
        fflush(stdout);
    }

    cont_ad_close(cont);
    fclose(rawfd);
}

/* Sleep for specified msec */
static void
sleep_msec(int32 ms)
{
    struct timeval tmo;

    tmo.tv_sec = 0;
    tmo.tv_usec = ms * 1000;

    select(0, NULL, NULL, NULL, &tmo);
}

/*
 * Main utterance processing loop:
 *     for (;;) {
 * 	   wait for start of next utterance;
 * 	   decode utterance until silence of at least 1 sec observed;
 * 	   print utterance result;
 *     }
 */
static void
recognize_from_microphone()
{
    ad_rec_t *ad;
    int16 adbuf[4096];
    int32 k, ts, rem;
    char const *hyp;
    char const *uttid;
    cont_ad_t *cont;
    char word[256];
	char c1[256], c2[256];

	int tracking = 0;
	int halted = 0;
	int LEFT = 0;
	int RIGHT = 1;
	int MOVE_CENT = 100; //1 meter
	int numwords;

	setlinebuf(stdout);

    if ((ad = ad_open_dev(cmd_ln_str_r(config, "-adcdev"),
                          (int)cmd_ln_float32_r(config, "-samprate"))) == NULL)
        E_FATAL("Failed to open audio device\n");

    /* Initialize continuous listening module */
    if ((cont = cont_ad_init(ad, ad_read)) == NULL)
        E_FATAL("Failed to initialize voice activity detection\n");
    if (ad_start_rec(ad) < 0)
        E_FATAL("Failed to start recording\n");
    if (cont_ad_calib(cont) < 0)
        E_FATAL("Failed to calibrate voice activity detection\n");


	printf("LEDON BLUE\n");
    for (;;) {
        /* Indicate listening for next utterance */
        fprintf(stderr, "READY....\n");
        fflush(stderr);

        /* Wait data for next utterance */
        while ((k = cont_ad_read(cont, adbuf, 4096)) == 0)
            sleep_msec(100);

        if (k < 0)
            E_FATAL("Failed to read audio\n");

        /*
         * Non-zero amount of data received; start recognition of new utterance.
         * NULL argument to uttproc_begin_utt => automatic generation of utterance-id.
         */
        if (ps_start_utt(ps, NULL) < 0)
            E_FATAL("Failed to start utterance\n");
        ps_process_raw(ps, adbuf, k, FALSE, FALSE);
        fprintf(stderr, "Listening...\n");

        /* Note timestamp for this first block of data */
        ts = cont->read_ts;

        /* Decode utterance until end (marked by a "long" silence, >1sec) */
        for (;;) {
            /* Read non-silence audio data, if any, from continuous listening module */
            if ((k = cont_ad_read(cont, adbuf, 4096)) < 0)
                E_FATAL("Failed to read audio\n");
            if (k == 0) {
                /*
                 * No speech data available; check current timestamp with most recent
                 * speech to see if more than 1 sec elapsed.  If so, end of utterance.
                 */
                if ((cont->read_ts - ts) > DEFAULT_SAMPLES_PER_SEC)
                    break;
            }
            else {
                /* New speech data received; note current timestamp */
                ts = cont->read_ts;
            }

            /*
             * Decode whatever data was read above.
             */
            rem = ps_process_raw(ps, adbuf, k, FALSE, FALSE);

            /* If no work to be done, sleep a bit */
            if ((rem == 0) && (k == 0))
                sleep_msec(20);
        }

        /*
         * Utterance ended; flush any accumulated, unprocessed A/D data and stop
         * listening until current utterance completely decoded
         */
        ad_stop_rec(ad);
        while (ad_read(ad, adbuf, 4096) >= 0);
        cont_ad_reset(cont);

        fprintf(stderr, "Stopped listening, please wait...\n");
        fflush(stdout);
        /* Finish decoding, obtain and print result */
        ps_end_utt(ps);
        hyp = ps_get_hyp(ps, NULL, &uttid);
        fprintf(stderr, "%s: %s\n", uttid, hyp);

        /* Exit if the first word spoken was GOODBYE */
        if (hyp) {
			numwords = sscanf(hyp, "%s %s %s", word, c1, c2);
			if(strcmp(word, "GUGGUG") == 0) {
				if(strcmp(c1, "HALT") == 0) {
					printf("LEDOFF BLUE\n");
					halted = 1;
				} else if(strcmp(c1, "RESUME") == 0) {
					printf("LEDON BLUE\n");
					halted = 0;
				}
				if(strcmp(c1, "BEGIN") == 0 || strcmp(c1, "START") == 0) {
					if(strcmp(c2, "TRACKING") == 0 && !tracking) {
						printf("START TRACKING\n");
						tracking = 1;
						halted = 0;
					}
				} else if(strcmp(c1, "STOP") == 0) {
					if(strcmp(c2, "TRACKING") == 0 && tracking) {
						printf("STOP TRACKING\n");
						tracking = 0;
					}
				}
				if(!tracking && !halted && numwords == 3) {
					if(strcmp(c1, "TURN") == 0) {
						if(strcmp(c2, "AROUND") == 0) {
							printf("TURN %d 180\n", LEFT);
						} else if(strcmp(c2, "LEFT") == 0) {
							printf("TURN %d 90\n", LEFT);
						} else if(strcmp(c2, "RIGHT") == 0) {
							printf("TURN %d 90\n", RIGHT);
						}
					} else if(strcmp(c1, "MOVE") == 0) {
						if(strcmp(c2, "FORWARD") == 0) {
							printf("MOVE 0 %d\n", MOVE_CENT);
						} else if(strcmp(c2, "BACKWARD") == 0) {
							printf("MOVE 1 %d\n", MOVE_CENT);
						}
					}
				}
			}
        }

        /* Resume A/D recording for next utterance */
        if (ad_start_rec(ad) < 0)
            E_FATAL("Failed to start recording\n");
    }

    cont_ad_close(cont);
    ad_close(ad);
}

static jmp_buf jbuf;
static void
sighandler(int signo)
{
    longjmp(jbuf, 1);
}

int
main(int argc, char *argv[])
{
    char const *cfg;
	int i;

	argc = 7;
	argv[1] = "-dict";
	argv[2] = "../../../../../../home/pi/guggug/speech/knowledgebase/3/dictionary";
	argv[3] = "-lm";
	argv[4] = "../../../../../../home/pi/guggug/speech/knowledgebase/3/langmodel";
	argv[5] = "-adcdev";
	argv[6] = "hw:1,0";

    if (argc == 2) {
        config = cmd_ln_parse_file_r(NULL, cont_args_def, argv[1], TRUE);
    }
    else {
        config = cmd_ln_parse_r(NULL, cont_args_def, argc, argv, FALSE);
    }
    /* Handle argument file as -argfile. */
    if (config && (cfg = cmd_ln_str_r(config, "-argfile")) != NULL) {
        config = cmd_ln_parse_file_r(config, cont_args_def, cfg, FALSE);
    }
    if (config == NULL)
        return 1;

    ps = ps_init(config);
    if (ps == NULL)
        return 1;

    E_INFO("%s COMPILED ON: %s, AT: %s\n\n", argv[0], __DATE__, __TIME__);

    if (cmd_ln_str_r(config, "-infile") != NULL) {
	recognize_from_file();
    } else {

        /* Make sure we exit cleanly (needed for profiling among other things) */
	/* Signals seem to be broken in arm-wince-pe. */
#if !defined(GNUWINCE) && !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
	signal(SIGINT, &sighandler);
#endif

        if (setjmp(jbuf) == 0) {
	    recognize_from_microphone();
	}
    }

    ps_free(ps);
    return 0;
}
