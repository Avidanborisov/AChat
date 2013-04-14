#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AChat.h"

typedef struct Client
{
	int sockfd;
	char name[MAX_NAME];
} Client;

int initServer(void);
int newConnection(int listener, fd_set* master, int* fdmax);
void makeResponse(char* msg, char* name);
void sendToAll(Client clients[], int clientsNum, char* msg);
void swapClients(Client* a, Client* b);

int main(void)
{
	Client clients[MAX_CLIENTS] = { { 0 } };
	int clientsNum = 0;
	int listener;
	char msg[MAX_RESPONSE] = "";
	fd_set master, read;
	int fdmax;

	startsock();

	listener = initServer();
	if (listen(listener, BACKLOG) == -1)
	{
		perror("server: listen");
		exit(3);
	}

	FD_ZERO(&master);
	FD_ZERO(&read);
	FD_SET(listener, &master);
	fdmax = listener;

	printf("server: waiting for connections...\n");

	while (1)
	{
		read = master;
		if (select(fdmax + 1, &read, NULL, NULL, NULL) == -1)
		{
			perror("server: select");
			exit(4);
		}

		if (FD_ISSET(listener, &read))
		{
			int client = newConnection(listener, &master, &fdmax);

			if (client != -1)
			{
				if (clientsNum < MAX_CLIENTS)
				{
					clients[clientsNum].sockfd = client;
					clients[clientsNum++].name[0] = '\0';
				}
				else
				{
					const char bye[] = "Too much clients. Try again later.\n";
					recv(client, msg, sizeof msg, 0);
					send(client, bye, sizeof bye, 0);
					close(client);
					FD_CLR(client, &master);
				}
			}
		}
		else
		{
			int i, bytes;

			for (i = 0; i < clientsNum; i++)
			{
				if (FD_ISSET(clients[i].sockfd, &read))
				{
					char* name = clients[i].name;
					bytes = recv(clients[i].sockfd, msg, MAX_MSG - 1, 0);
					msg[bytes >= 0 ? bytes : 0] = '\0';

					if (strcmp(msg, "/exit\n") == 0)
					{
						bytes = 0;
						msg[0] = '\0';
					}

					if (bytes <= 0)
					{
						close(clients[i].sockfd);
						FD_CLR(clients[i].sockfd, &master);
						clientsNum--;
						swapClients(&clients[i], &clients[clientsNum]);
						name = clients[clientsNum].name;
					}

					if (bytes >= 0)
					{
						makeResponse(msg, name);
						printf("%s", msg);
						sendToAll(clients, clientsNum, msg);
					}
				}
			}
		}
	}

	close(listener);
	endsock();

	return 0;
}

int initServer(void)
{
	int rc, sockfd;
	struct addrinfo hints = { 0 };
	struct addrinfo* server;
	struct addrinfo* p;

	hints.ai_family = IP_FAMILY;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rc = getaddrinfo(NULL, PORT, &hints, &server)) != 0)
	{
		fprintf(stderr, "server: %s\n", gai_strerror(rc));
		exit(1);
	}

	for (p = server; p != NULL; p = p->ai_next)
	{
		sockopt_t yes = 1;

		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
		{
			perror("server: setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)
	{
		fprintf(stderr, "server: failed to bind\n");
		exit(2);
	}

	freeaddrinfo(server);

	return sockfd;
}

int newConnection(int listener, fd_set* master, int* fdmax)
{
	int sockfd;
	struct sockaddr_storage clientAddr;
	socklen_t addrlen = sizeof clientAddr;
	char IP[INET6_ADDRSTRLEN];

	sockfd = accept(listener, (struct sockaddr *)&clientAddr, &addrlen);
	if (sockfd == -1)
	{
		perror("server: accept");
	}
	else
	{
		FD_SET(sockfd, master);
		if (sockfd > *fdmax)
		{
			*fdmax = sockfd;
		}

		getnameinfo((struct sockaddr *)&clientAddr, addrlen, IP, sizeof IP,
					NULL, 0, NI_NUMERICHOST);
		printf("server: new connection from %s on socket %d\n", IP, sockfd);
	}

	return sockfd;
}

void makeResponse(char* msg, char* name)
{
	if (name[0] == '\0') /* Client joines */
	{
		strcpy(name, msg);
		strcat(msg, " joined.\n");
	}
	else if (msg[0] == '\0') /* Client leaves */
	{
		strcpy(msg, name);
		strcat(msg, " left.\n");
	}
	else /* Client says something */
	{
		char temp[MAX_RESPONSE];
		sprintf(temp, "<%s> %s", name, msg);
		strcpy(msg, temp);
	}
}

void sendToAll(Client clients[], int clientsNum, char* msg)
{
	int j;
	size_t len = strlen(msg) + 1;

	for (j = 0; j < clientsNum; j++)
	{
		if (send(clients[j].sockfd, msg, len, 0) == -1)
		{
			perror("server: send");
		}
	}
}

void swapClients(Client* a, Client* b)
{
	Client temp = *a;
	*a = *b;
	*b = temp;
}
