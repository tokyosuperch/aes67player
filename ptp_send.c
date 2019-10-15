#include <stdlib.h>
#include <stdbool.h>

extern int seqid;

char *ptpmsg();
char ptpflags(bool PTP_LI61, bool PTP_LI59, bool PTP_BOUNDARY_CLOCK, bool PTP_ASSIST, bool PTP_EXT_SYNC, bool PTP_PARENT_STATS, bool PTP_SYNC_BURST);

char *ptpmsg() {
	char *temp;
	temp = (char *)malloc(124 * sizeof(char));
	// versionPTP この関数はPTPv1用
	temp[0x00] = (char)0x00;
	temp[0x01] = (char)0x01;
	// versionNetwork
	temp[0x02] = (char)0x00;
	temp[0x03] = (char)0x01;
	// subdomain _DFLT
	temp[0x04] = '_';
	temp[0x05] = 'D';
	temp[0x06] = 'F';
	temp[0x07] = 'L';
	temp[0x08] = 'T';
	for (int i = 0x09; i < 0x14; i++) temp[i] = '\0';
	// messageType スレーブで使う時はEventMessage(1)のみ？
	temp[0x14] = (char)0x01;
	// sourceCommunicationTechnology
	// IEEE 802.3(Ethernet)(1)
	temp[0x15] = (char)0x01;
	// sourceUuid 保留
	// sourcePortId 1
	temp[0x1c] = (char)0x00;
	temp[0x1d] = (char)0x01;
	// sequenceId
	temp[0x1e] = (char)(seqid >> 8);
	temp[0x1f] = (char)(seqid % 256);
	// control Delay_Req Message(1)
	temp[0x20] = (char)0x01;
	// 不明
	temp[0x21] = (char)0x00;
	// flags
	temp[0x22] = (char)0x00;
	temp[0x23] = ptpflags(false, false, true, true, false, false, false);
	return temp;
}

char ptpflags(bool PTP_LI61, bool PTP_LI59, bool PTP_BOUNDARY_CLOCK, bool PTP_ASSIST, bool PTP_EXT_SYNC, bool PTP_PARENT_STATS, bool PTP_SYNC_BURST) {
	char temp = (char)0;
	temp += (char)PTP_SYNC_BURST;
	temp = temp << 1;
	temp += (char)PTP_PARENT_STATS;
	temp = temp << 1;
	temp += (char)PTP_EXT_SYNC;
	temp = temp << 1;
	temp += (char)PTP_ASSIST;
	temp = temp << 1;
	temp += (char)PTP_BOUNDARY_CLOCK;
	temp = temp << 1;
	temp += (char)PTP_LI59;
	temp = temp << 1;
	temp += (char)PTP_LI61;
	return temp;
}
