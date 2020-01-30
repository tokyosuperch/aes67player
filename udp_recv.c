#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

/*
 *  マルチキャスト情報
 */
struct multicast_info {
    char *ipaddr;            /* マルチキャストのIPアドレス */
    unsigned short port;     /* ポート番号 */

    int sd;                  /* ソケットディスクリプタ */
    struct sockaddr_in addr; /* マルチキャストのアドレス構造体 */
};
typedef struct multicast_info mc_info_t;

#define MAXRECVSTRING 255  /* Longest string to receive */

static int initialize(int argc, char *argv[], mc_info_t *info, char *errmsg, int port);
static int multicast_receiver(mc_info_t *info, char *errmsg);
static int socket_initialize(mc_info_t *info, char *errmsg);
static int multicast_receive(mc_info_t *info, char *errmsg);
static void socket_finalize(mc_info_t *info);
extern int ptp_recv(unsigned char* msg);
extern void *sendapp();
extern struct clockinfo grandmaster;

// モード
// 0: 待機
// 1: Syncを受け取りFollow_Up待ち
// 2: Follow_Upを受け取りDelay_Request送信中
// 3: Delay_Requestを受け取りSync_Message待ち
int mode = 0;

/*!
 * @brief   main routine
 * @return  成功ならば0、失敗ならば-1を返す。
 */
int main(int argc, char *argv[]) {
	pthread_t pthread;
	// printf("\nMake sure that you disabled NTP!\n\n");
	for (;;) {
		int rc = 0;
		mc_info_t info = {0};
		/* char errmsg[BUFSIZ];
		if (mode >= 1 && mode <= 3) {
			rc = initialize(argc, argv, &info, errmsg, 320);
		} else {
			rc = initialize(argc, argv, &info, errmsg, 319);
		} */
		/* if(rc != 0){ 
			fprintf(stderr, "Error: %s\n", errmsg);
			return(-1);
		} */
		sendapp();
		/* if (mode == 2) {
			pthread_create( &pthread, NULL, &sendapp, NULL );
			mode = 3;
		} */
		/* rc = multicast_receiver(&info, errmsg);
		if(rc != 0){ 
			fprintf(stderr, "Error: %s\n", errmsg);
			return(-1);
		} */
	}
	return(0);
}

/*!
 * @brief      初期化処理。
 * @param[in]  argc   コマンドライン引数の数
 * @param[in]  argv   コマンドライン引数
 * @param[out] info   マルチキャスト情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int initialize(int argc, char *argv[], mc_info_t *info, char *errmsg, int port) {
    /* if(argc != 3){
        sprintf(errmsg, "usage: %s <multicast address> <port>", argv[0]);
        return(-1);
    } */

    memset(info, 0, sizeof(mc_info_t));
    info->ipaddr = "224.0.1.129";
    info->port   = port;

    return(0);
}

/*!
 * @brief      マルチキャストを受信する
 * @param[in]  info   マルチキャスト情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int multicast_receiver(mc_info_t *info, char *errmsg) {
    int rc = 0;

    /* ソケットの初期化 */
    rc = socket_initialize(info, errmsg);
    if(rc != 0) return(-1);

    /* マルチキャストを送信する */
    rc = multicast_receive(info, errmsg);

    /* ソケットの終期化 */
    socket_finalize(info);

    return(rc);
}

/*!
 * @brief      ソケットの初期化
 * @param[in]  info   マルチキャスト情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int socket_initialize(mc_info_t *info, char *errmsg) {
    int rc = 0;
    struct ip_mreq multicast_request;

    /* ソケットの生成 : UDPを指定する */
    info->sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(info->sd < 0){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return(-1);
    }

    /* アドレス構造体を作成する */
    info->addr.sin_family = AF_INET;
    info->addr.sin_addr.s_addr = htonl(INADDR_ANY);
    info->addr.sin_port = htons(info->port);

    /* ポートにバインドする*/
    rc = bind(info->sd, (struct sockaddr *)&(info->addr),
              sizeof(info->addr));
    if(info->sd < 0){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return(-1);
    }

    /* マルチキャストグループの設定を行う */
    multicast_request.imr_multiaddr.s_addr = inet_addr(info->ipaddr);
    multicast_request.imr_interface.s_addr = htonl(INADDR_ANY);
    rc = setsockopt(info->sd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                    (void *)&multicast_request, sizeof(multicast_request));
    if(rc < 0){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return(-1);
    }

    return(0);
}

/*!
 * @brief      マルチキャストを受信する
 * @param[in]  info   マルチキャスト情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int multicast_receive(mc_info_t *info, char *errmsg) {
    unsigned char recv_msg[MAXRECVSTRING+1];
    int recv_msglen = 0;

    /* Receive a single datagram from the server */
    recv_msglen = recvfrom(info->sd, recv_msg, MAXRECVSTRING, 0, NULL, 0);
    if(recv_msglen < 0){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return(-1);
    }

    recv_msg[recv_msglen] = '\0';
    // printf("Received: %s\n", recv_msg);    /* Print the received string */
    ptp_recv(recv_msg);
    return(0);
}

/*!
 * @brief      ソケットの終期化
 * @param[in]  info   マルチキャスト情報
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static void socket_finalize(mc_info_t *info) {
    /* ソケット破棄 */
    if(info->sd != 0) close(info->sd);

    return;
}

