/*
 * JFAud Red-Book CD Audio playback for Windows
 * Copyright (C) 2005 Jonathon Fowler
 */

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <mmsystem.h>
#include "cda.h"


static MCIDEVICEID deviceid = 0;
static int pausepos = -1, lastplayendtime = -1;

cdatoc cda_trackinfo[MAX_CDA_TRACKS];
int    cda_numtracks = 0;


static int sendCommand(UINT msg, DWORD flags, DWORD param)
{
	MCIERROR err;
	TCHAR errmsg[128];

	err = mciSendCommand(deviceid, msg, flags, param);
	if (err == 0) return 0;

	if (mciGetErrorString(LOWORD(err), errmsg, 128))
		printf("MCI Error: %s\n", errmsg);

	return LOWORD(err);
}

static int toMSF(int frames)
{
	int m,s,f;

	f = frames % CDA_FPS;
	s = (frames / CDA_FPS) % 60;
	m = (frames / CDA_FPS) / 60;

	return MCI_MAKE_MSF(m,s,f);
}


int cda_opendevice(void)
{
	MCI_OPEN_PARMS mop;
	MCI_SET_PARMS  msp;

	pausepos = lastplayendtime = -1;

	if (deviceid) return -1;

	memset(&mop, 0, sizeof(mop));
	mop.lpstrDeviceType = (LPCSTR)MCI_DEVTYPE_CD_AUDIO;

	if (sendCommand(MCI_OPEN, MCI_WAIT | MCI_OPEN_SHAREABLE | MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID, (DWORD)&mop)) return -1;

	deviceid = mop.wDeviceID;

	memset(&msp, 0, sizeof(msp));
	msp.dwTimeFormat = MCI_FORMAT_MSF;

	if (sendCommand(MCI_SET, MCI_WAIT | MCI_SET_TIME_FORMAT, (DWORD)&msp)) return -1;

	return 0;
}

int cda_closedevice(void)
{
	MCI_GENERIC_PARMS mgp;

	if (!deviceid) return -1;

	memset(&mgp, 0, sizeof(mgp));

	if (sendCommand(MCI_CLOSE, MCI_WAIT, (DWORD)&mgp)) return -1;

	deviceid = 0;
	
	return 0;
}

int cda_getstatus(void)
{
	MCI_STATUS_PARMS msp;

	if (!deviceid) return -1;

	memset(&msp, 0, sizeof(msp));
	msp.dwItem = MCI_STATUS_MODE;

	if (sendCommand(MCI_STATUS, MCI_WAIT | MCI_STATUS_ITEM, (DWORD)&msp)) return -1;

	switch (msp.dwReturn) {
		case MCI_MODE_NOT_READY: return CDA_NotReady;
		case MCI_MODE_PLAY:      return CDA_Playing;
		case MCI_MODE_STOP:      if (pausepos >= 0) return CDA_Paused; return CDA_Stopped;
		case MCI_MODE_OPEN:      return CDA_NotReady;
		case MCI_MODE_SEEK:      return CDA_Seeking;
		default: return -1;
	}
}

int cda_querydisc(void)
{
	int i;
	MCI_STATUS_PARMS msp;

	if (!deviceid) return -1;

	cda_numtracks = 0;

	memset(&msp, 0, sizeof(msp));
	msp.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;

	if (sendCommand(MCI_STATUS, MCI_WAIT | MCI_STATUS_ITEM, (DWORD)&msp)) return -1;

	cda_numtracks = msp.dwReturn;

	memset(&msp, 0, sizeof(msp));
	for (i=0; i<cda_numtracks; i++) {
		msp.dwTrack = i+1;

		msp.dwItem = MCI_STATUS_POSITION;
		if (sendCommand(MCI_STATUS, MCI_WAIT | MCI_STATUS_ITEM | MCI_TRACK, (DWORD)&msp)) {
			cda_numtracks = 0;
			return -1;
		}

		cda_trackinfo[i].start  = MCI_MSF_FRAME(msp.dwReturn);
		cda_trackinfo[i].start += MCI_MSF_SECOND(msp.dwReturn)*CDA_FPS;
		cda_trackinfo[i].start += MCI_MSF_MINUTE(msp.dwReturn)*CDA_FPS*60;

		msp.dwItem = MCI_STATUS_LENGTH;
		if (sendCommand(MCI_STATUS, MCI_WAIT | MCI_STATUS_ITEM | MCI_TRACK, (DWORD)&msp)) {
			cda_numtracks = 0;
			return -1;
		}

		cda_trackinfo[i].length  = MCI_MSF_FRAME(msp.dwReturn);
		cda_trackinfo[i].length += MCI_MSF_SECOND(msp.dwReturn)*CDA_FPS;
		cda_trackinfo[i].length += MCI_MSF_MINUTE(msp.dwReturn)*CDA_FPS*60;

		msp.dwItem = MCI_CDA_STATUS_TYPE_TRACK;
		if (sendCommand(MCI_STATUS, MCI_WAIT | MCI_STATUS_ITEM | MCI_TRACK, (DWORD)&msp)) {
			cda_numtracks = 0;
			return -1;
		}

		cda_trackinfo[i].isdata = (msp.dwReturn == MCI_CDA_TRACK_OTHER);
	}

	return 0;
}

int cda_play(int startframe, int endframe)
{
	DWORD flags;
	MCI_PLAY_PARMS mpp;

	if (!deviceid) return -1;

	flags = 0;
	memset(&mpp, 0, sizeof(mpp));

	if (startframe >= 0) flags |= MCI_FROM, mpp.dwFrom = toMSF(startframe);
	if (endframe >= 0)   flags |= MCI_TO,   mpp.dwTo   = toMSF(endframe);

	if (sendCommand(MCI_PLAY, flags, (DWORD)&mpp)) return -1;

	lastplayendtime = endframe;
	pausepos = -1;

	return 0;
}

int cda_stop(void)
{
	DWORD flags;
	MCI_GENERIC_PARMS mgp;

	if (!deviceid) return -1;

	memset(&mgp, 0, sizeof(mgp));

	if (sendCommand(MCI_STOP, 0, (DWORD)&mgp)) return -1;
	pausepos = lastplayendtime = -1;

	return 0;
}

int cda_pause(int on)
{
	if (!deviceid) return -1;

	if (!on) {
		MCI_PLAY_PARMS mpp;

		if (pausepos < 0) return 0;	// wasn't paused

		memset(&mpp, 0, sizeof(mpp));
		mpp.dwFrom = pausepos;
		if (lastplayendtime >= 0) mpp.dwTo = toMSF(lastplayendtime);

		if (sendCommand(MCI_PLAY, MCI_FROM | (lastplayendtime<0?0:MCI_TO), (DWORD)&mpp)) return -1;

		pausepos = -1;
		return 0;
	} else {
		MCI_STATUS_PARMS msp;
		MCI_GENERIC_PARMS mgp;

		if (pausepos >= 0) return 0;	// paused already

		memset(&msp, 0, sizeof(msp));
		msp.dwItem = MCI_STATUS_POSITION;
		if (sendCommand(MCI_STATUS, MCI_WAIT | MCI_STATUS_ITEM, (DWORD)&msp)) return -1;
		pausepos = msp.dwReturn;

		memset(&mgp, 0, sizeof(mgp));
		if (sendCommand(MCI_STOP, 0, (DWORD)&mgp)) return -1;
	}

	return 0;
}


