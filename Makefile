all: udp_recv.c ptp_recv.c udp_send.c ptp_send.c
	gcc -pthread -Wall -o ptp.elf udp_recv.c ptp_recv.c udp_send.c ptp_send.c
