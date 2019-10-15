#include <stdio.h>
#define SDOMAIN_LEN 16
#define UUID_LEN 6


char srcUuid[UUID_LEN];
int srcPort;
int srcSeqId;

int ptp_recv(unsigned char* msg) {
	// versionPTP 0x00-0x01
	// この関数はPTPv1用
	if (msg[0x00] == (char)0 && msg[0x01] == (char)1) {
		printf("Received PTPv1 Message!\n");
	} else {
		printf("This is not a PTPv1 Message!\n");
		return -1;
	}
	// versionNetwork 0x02-0x03
	// subdomain _DFLT
	char subdomain[SDOMAIN_LEN];
	for (int i = 0; i < SDOMAIN_LEN; i++) subdomain[i] = msg[i + 0x04]; 
	printf("%s\n", subdomain);
	// messageType 0x14
	int msgType = (int)msg[0x14];
	// sourceCommunicationTechnology 0x15
	if (msg[0x15] != 1) {
		printf("This works only in ethernet!\n");
		return -1;
	}
	// sourceUuid 0x16-0x1B
	for (int i = 0; i < UUID_LEN; i++) srcUuid[i] = msg[i + 0x16];
	// sourcePort 0x1B-0x1C
	srcPort = (int)msg[0x1c];
	srcPort = srcPort << 8;
	srcPort += (int)msg[0x1d];
	// sequenceId 0x1D-0x1E
	srcSeqId = (int)msg[0x1e];
	srcSeqId = srcSeqId << 8;
	srcSeqId += (int)msg[0x1f];
	printf("Source Sequence Id: %d\n", srcSeqId);
	return 0;
}
