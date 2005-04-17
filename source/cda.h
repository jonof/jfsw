enum {
	CDA_NotReady,
	CDA_Paused,
	CDA_Playing,
	CDA_Stopped,
	CDA_Seeking
};

typedef struct {
	unsigned long start, length;
	char isdata;
} cdatoc;
#define MAX_CDA_TRACKS 99	// an audio CD can only have 99 tracks as it is
#define CDA_FPS 75

extern cdatoc cda_trackinfo[MAX_CDA_TRACKS];
extern int    cda_numtracks;

int cda_opendevice(void);
int cda_closedevice(void);
int cda_getstatus(void);
int cda_querydisc(void);
int cda_play(int startframe, int endframe);
int cda_stop(void);
int cda_pause(int on);

