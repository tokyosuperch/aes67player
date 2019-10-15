#include <stdio.h>

int ptp_recv(char* msg) {
	// versionPTP この関数はPTPv1用
	if (msg[0x00] == (char)0 && msg[0x01] == (char)1) {
		printf("Received PTPv1 Message!\n");
	} else {
		printf("This is not a PTPv1 Message!\n");
		return -1;
	}
	// subdomain _DFLT
	printf("%c%c%c%c%c\n", msg[0x04], msg[0x05], msg[0x06], msg[0x07], msg[0x08]);
	return 0;
}
