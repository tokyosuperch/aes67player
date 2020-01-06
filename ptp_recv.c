#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "info.h"
#define SDOMAIN_LEN 16
#define UUID_LEN 6


unsigned char srcUuid[UUID_LEN];
unsigned int srcPort;
unsigned int srcSeqId;
struct timespec sync_ts;
struct timespec ts;

int sync_msg(unsigned char* msg);
int followup_msg(unsigned char* msg);
unsigned long long int charToInt(int bytes, ...);
extern int mode;
char subdomain[SDOMAIN_LEN];
struct clockinfo grandmaster;

int ptp_recv(unsigned char* msg) {

	printf("Receiving...\n");

	int ret = 0;
	// versionPTP 0x00-0x01
	// この関数はPTPv1用
	if (msg[0x00] == (char)0 && msg[0x01] == (char)1) {
		// printf("Received PTPv1 Message!\n");
	} else {
		// printf("This is not a PTPv1 Message!\n");
		return -1;
	}
	// versionNetwork 0x02-0x03
	// subdomain _DFLT
	subdomain[SDOMAIN_LEN];
	for (int i = 0; i < SDOMAIN_LEN; i++) subdomain[i] = msg[i + 0x04]; 
	// printf("%s\n", subdomain);
	// messageType 0x14
	int msgType = (int)msg[0x14];
	// sourceCommunicationTechnology 0x15
	if (msg[0x15] != 1) {
		printf("This works only in ethernet!\n");
		return -1;
	}
	// sourceUuid 0x16-0x1B
	// sourcePort 0x1C-0x1D
	srcPort = (int)charToInt(2, msg[0x1c], msg[0x1d]);
	// sequenceId 0x1E-0x1F
	srcSeqId = (int)charToInt(2, msg[0x1e], msg[0x1f]);
	// control 0x20
	printf("msgType=%d, control=%d\n", msgType, (int)msg[0x20]);
	if (msg[0x20] == 0x00 && msgType == 1) {
		// Sync Message
		ret = sync_msg(msg);
	} else if (msg[0x20] == 0x02 && msgType == 2) {
		ret = followup_msg(msg);
	} else if (msg[0x20] == 0x03 && msgType == 2) {
		printf("Received Delay Response Message!\n");
		fflush(stdout);
		sleep(1);
		mode = 0;
	} else {
		// 未実装
		// printf("未実装 messageType:%d control:%d\n", msgType, msg[0x20]);
		return -1;
	}
	return ret;
}

int sync_msg(unsigned char* msg) {
	if (mode != 0) return -1;
	// sourceUuid 0x16-0x1B
	for (int i = 0; i < UUID_LEN; i++) srcUuid[i] = msg[i + 0x16];
	// flags 0x22-0x23
	// originTimeStamp 0x28-0x2F
	// (seconds) 0x28-0x2B
	sync_ts.tv_sec = charToInt(4, msg[0x28], msg[0x29], msg[0x2a], msg[0x2b]);
	// (nanoseconds) 0x2C-0x2F
	sync_ts.tv_nsec = charToInt(4, msg[0x2c], msg[0x2d], msg[0x2e], msg[0x2f]);
	// epochNumber 0x30-0x31
	// currentUTCOffset 0x32-0x33
	// grandmasterCommunicationTechnology 0x35
	grandmaster.CommunicationTechnology = msg[0x35];
	// grandmasterClockUuid 0x36-0x3B
	for (int i = 0; i < UUID_LEN; i++) grandmaster.ClockUuid[i] = msg[0x36+i];
	// grandmasterPortId 0x3C-0x3D
	grandmaster.PortId = charToInt(2, msg[0x3c], msg[0x3d]);
	// grandmasterSequenceId 0x3E-0x3F
	grandmaster.SequenceId = charToInt(2, msg[0x3e], msg[0x3f]);
	// grandmasterClockStratum 0x43
	grandmaster.ClockStratum = msg[0x43];
	// grandmasterClockIdentifier 0x44-0x47
	for (int i = 0; i < 4; i++) grandmaster.ClockIdentifier[i] = (char)msg[0x44+i];
	// grandmasterClockVariance 0x4A-0x4B
	// charToInt()はunsignedのためめんどくさい方法を取りました
	char lendian[2];
	lendian[0] = msg[0x4b];
	lendian[1] = msg[0x4a];
	memcpy(&grandmaster.ClockVariance, &lendian, 2);
	// grandmasterPreferred 0x4D
	grandmaster.Preferred = msg[0x4d];
	// grandmasterIsBoundaryClock 0x4F
	grandmaster.IsBoundaryClock = msg[0x4f];
	clock_gettime(CLOCK_REALTIME, &ts);
	printf("originTimeStamp: %ld.%ld seconds\n", sync_ts.tv_sec, sync_ts.tv_nsec);
	printf("tv_sec=%ld  tv_nsec=%ld\n\n",ts.tv_sec,ts.tv_nsec);
	mode = 1;
	return 0;
}

int followup_msg(unsigned char* msg) {
	for (int i = 0; i < UUID_LEN; i++) {
		if (msg[i + 0x16] != srcUuid[i]) {
			printf("%d, %d Missmatch\n", (int)msg[i + 0x16], (int)srcUuid[i]);
			return -1;
		}
	}
	// associatedSequenceId 0x2a-0x2b
	mode = 2;
	return 0;
}

unsigned long long int charToInt(int bytes, ...) {
	va_list list;
	unsigned long long int temp = 0; 

	va_start(list, bytes);
	for(int i = 0; i < bytes; i++) {
		temp += va_arg(list, int);
		if (bytes - i > 1) temp = temp << 8;
	}
	va_end(list);

	return temp;
}
