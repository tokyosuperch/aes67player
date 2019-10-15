all: udp_recv.c ptp_recv.c udp_send.c ptp_send.c
	gcc -Wall -o recv.elf udp_recv.c ptp_recv.c
	gcc -Wall -o send.elf udp_send.c ptp_send.c
recv: udp_recv.c ptp_recv.c
	gcc -Wall -o recv.elf udp_recv.c ptp_recv.c
send: udp_send.c ptp_send.c
	gcc -Wall -o send.elf udp_send.c ptp_send.c
