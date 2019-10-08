#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

int main(int argc, char** argv)
{
    int sd;
    struct sockaddr_in addr;
 
    if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return -1;
    }
 
    // 送信先アドレスとポート番号を設定する
    // 受信プログラムと異なるあて先を設定しても UDP の場合はエラーにはならない
    addr.sin_family = AF_INET;
    addr.sin_port = htons(319);
    addr.sin_addr.s_addr = inet_addr("224.0.1.129");
    if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
       printf("%s\n", strerror(errno));
    }

    // パケットをUDPで送信
    if(sendto(sd, "I am send process", 17, 0,
              (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("sendto");
        return -1;
    }
 
    close(sd);
 
    return 0;
}
