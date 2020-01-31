#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <time.h>
#include "info.h"

extern char *ptpmsg();
extern int mode;
struct ifreq ifr;
unsigned char srcUuid[6];
extern struct timespec ts;
extern struct clockinfo grandmaster;
struct clockinfo gminit();
int isinit = 0;

void sendapp()
{
	int sd;
	struct sockaddr_in addr;

	if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		close(sd);
		return 0;
	}

	strncpy(ifr.ifr_name, "enp3s0", IFNAMSIZ-1);
	ioctl(sd, SIOCGIFHWADDR, &ifr);
	for (int i = 0; i < 6; i++) srcUuid[i] = ifr.ifr_hwaddr.sa_data[i];
	/* printf("%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
         (unsigned char)ifr.ifr_hwaddr.sa_data[0],
         (unsigned char)ifr.ifr_hwaddr.sa_data[1],
         (unsigned char)ifr.ifr_hwaddr.sa_data[2],
         (unsigned char)ifr.ifr_hwaddr.sa_data[3],
         (unsigned char)ifr.ifr_hwaddr.sa_data[4],
         (unsigned char)ifr.ifr_hwaddr.sa_data[5]); */
	if (isinit == 0) grandmaster = gminit();
	// 送信先アドレスとポート番号を設定する
	// 受信プログラムと異なるあて先を設定しても UDP の場合はエラーにはならない
	addr.sin_family = AF_INET;
	addr.sin_port = htons(320);
	addr.sin_addr.s_addr = inet_addr("224.0.1.129");
	bind(sd, (struct sockaddr *)&addr, sizeof(addr));
	/* if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
		printf("%s\n", strerror(errno));
	} */
	if (mode == 0) {
		// Sync MessageなどのEvent Message
		addr.sin_port = htons(319);
	}

	// パケットをUDPで送信
	if(sendto(sd, ptpmsg(), 124, 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("sendto");
		close(sd);
		return 0;
	}
	close(sd);
	// printf("send Delay Req msg\n");
	// fflush(stdout);

	return 0;
}

struct clockinfo gminit() {
	struct clockinfo temp;
	temp.CommunicationTechnology = (unsigned char)0x01; // Ethernet(1);
	strcpy((char *)temp.ClockUuid, (char *)srcUuid);
	temp.ClockStratum = (unsigned char)2;
	temp.PortId = (unsigned int)320;
	temp.SequenceId = (unsigned int)0;
	temp.ClockIdentifier[0] = 'N';
	temp.ClockIdentifier[1] = 'T';
	temp.ClockIdentifier[2] = 'P';
	temp.ClockIdentifier[3] = '\0';
	temp.ClockVariance = -4000;
	temp.Preferred = (unsigned char)0;
	temp.IsBoundaryClock = (unsigned char)0x01;
	isinit = 1;
	return temp;
}
