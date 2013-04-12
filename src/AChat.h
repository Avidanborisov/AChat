#ifndef ACHAT_H_
#define ACHAT_H_

#ifdef _WIN32

	#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x6000
	#endif
	#include <winsock2.h>
	#include <ws2tcpip.h>

	#define startsock()                                \
		WSADATA wsaData;                               \
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) \
		{                                              \
			fprintf(stderr, "WSAStartup failed.\n");   \
			exit(1);                                   \
		}                                              \

	#define endsock() WSACleanup();
	#define close closesocket
	typedef char sockopt_t;

#else

	#include <unistd.h>
	#include <errno.h>
	#include <netdb.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>

	#define startsock()
	#define endsock()
	typedef int sockopt_t;

#endif

#define IP_FAMILY    AF_INET
#define PORT         "1337"
#define MAX_CLIENTS  100
#define BACKLOG      10
#define MAX_NAME     20
#define MAX_MSG      1000
#define MAX_RESPONSE (MAX_NAME + MAX_MSG + sizeof "<> ")

#endif /* ACHAT_H_ */
