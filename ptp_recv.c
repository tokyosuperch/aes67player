#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#define SDOMAIN_LEN 16
#define UUID_LEN 6


char srcUuid[UUID_LEN];
int srcPort;
int srcSeqId;
struct timespec sync_ts;
struct timespec ts;
// モード
// 0: 待機
// 1: Syncを受け取りFollow_Up待ち
// 2: Follow_Upを受け取りDelay_Request送信中
// 3: Delay_Requestを受け取りSync_Message待ち
int mode = 0;

int sync_msg(unsigned char* msg);
int followup_msg(unsigned char* msg);
unsigned long long int charToInt(int bytes, ...);

int ptp_recv(unsigned char* msg) {
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
	char subdomain[SDOMAIN_LEN];
	for (int i = 0; i < SDOMAIN_LEN; i++) subdomain[i] = msg[i + 0x04]; 
	// printf("%s\n", subdomain);
	// messageType 0x14
	int msgType = (int)msg[0x14];
	// sourceCommunicationTechnology 0x15
	if (msg[0x15] != 1) {
		// printf("This works only in ethernet!\n");
		return -1;
	}
	// sourceUuid 0x16-0x1B
	// sourcePort 0x1C-0x1D
	srcPort = (int)charToInt(2, msg[0x1c], msg[0x1d]);
	// sequenceId 0x1E-0x1F
	srcSeqId = (int)charToInt(2, msg[0x1e], msg[0x1f]);
	// control 0x20
	if (msg[0x20] == 0x00 && msgType == 1) {
		// Sync Message
		ret = sync_msg(msg);
	} else if (msg[0x20] == 0x01 && msgType == 1) {
		ret = followup_msg(msg);
	} else {
		// 未実装
		// printf("未実装 messageType:%d control:%d\n", msgType, msg[0x20]);
		return -1;
	}
	return ret;
}

int sync_msg(unsigned char* msg) {
	// if (mode != 0) return -1;
	// sourceUuid 0x16-0x1B
	for (int i = 0; i < UUID_LEN; i++) srcUuid[i] = msg[i + 0x16];
	// flags 0x22-0x23
	// originTimeStamp 0x28-0x2F
	// (seconds) 0x28-0x2B
	sync_ts.tv_sec = charToInt(4, msg[0x28], msg[0x29], msg[0x2a], msg[0x2b]);
	// (nanoseconds) 0x2C-0x2F
	sync_ts.tv_nsec = charToInt(4, msg[0x2c], msg[0x2d], msg[0x2e], msg[0x2f]);
	clock_gettime(CLOCK_REALTIME, &ts);
	printf("originTimeStamp: %ld.%ld seconds\n", sync_ts.tv_sec, sync_ts.tv_nsec);
	printf("tv_sec=%ld  tv_nsec=%ld\n\n",ts.tv_sec,ts.tv_nsec);
	mode = 1;
	return 0;
}

int followup_msg(unsigned char* msg) {
	for (int i = 0; i < UUID_LEN; i++) {
		if (msg[i + 0x16] != srcUuid[i]) return -1;
	}
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
