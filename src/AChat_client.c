#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "AChat.h"

WINDOW* in;
WINDOW* out;

int connectToHost(const char* host);
void chatter(int sockfd);
void* sender(void* pfd);
WINDOW* newWindow(int y, int x, int height, int width, int color);
void printBox(WINDOW* win, int color);

int main(int argc, char* argv[])
{
	int sockfd;
	int len;

	startsock();

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s <host> <username>\n", argv[0]);
		return 1;
	}

	if ((len = strlen(argv[2]) + 1) > MAX_NAME)
	{
		fprintf(stderr, "client: username is too long\n");
		return 2;
	}

	sockfd = connectToHost(argv[1]);

	if (send(sockfd, argv[2], len, 0) == -1)
	{
		perror("client: failed to send name");
		return 5;
	}

	chatter(sockfd);
	endsock();

	return 0;
}

void chatter(int sockfd)
{
	char msg[MAX_RESPONSE];
	int bytes;
	int rows, cols;
	int outSize, inSize;

	pthread_t sendt;
	pthread_attr_t attr;

	initscr();
	cbreak();
	keypad(in, FALSE);
	start_color();
	use_default_colors();
	init_pair(1, COLOR_CYAN, -1);
	init_pair(2, COLOR_YELLOW, -1);

	getmaxyx(stdscr, rows, cols);
	outSize = rows - 5;
	inSize = rows - outSize - 1;

	out = newWindow(0, 0, outSize, cols - 2, COLOR_PAIR(1) | A_BOLD);
	in = newWindow(outSize + 1, 0, inSize - 1, cols - 2, COLOR_PAIR(2));

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&sendt, NULL, sender, &sockfd);
	pthread_attr_destroy(&attr);

	while (1)
	{
		bytes = recv(sockfd, msg, sizeof msg - 1, 0);

		if (bytes != 0)
		{
			if (bytes == -1)
			{
				wprintw(out, "Error receiving message: %s\n", strerror(errno));
			}
			else
			{
				msg[bytes] = '\0';
				wprintw(out, "%s", msg);
			}

			wrefresh(out);
		}
		else
		{
			break;
		}
	}

	pthread_cancel(sendt);
	pthread_detach(sendt);
	close(sockfd);
	wprintw(out, "Connection closed.\n");
	wprintw(out, "Press any key to exit.\n");
	wrefresh(out);
	wgetch(in);
	delwin(in);
	delwin(out);
	endwin();
}

void* sender(void* pfd)
{
	char msg[MAX_MSG] = "";
	int sockfd = *(int *)pfd;

	while (1)
	{
		mvwgetnstr(in, 0, 0, msg, sizeof msg - 2);
		strcat(msg, "\n");

		if (send(sockfd, msg, strlen(msg) + 1, 0) == -1)
		{
			wprintw(out, "Error sending message: %s\n", strerror(errno));
		}

		werase(in);
	}

	return NULL;
}

int connectToHost(const char* host)
{
	int rc, sockfd;
	struct addrinfo hints = { 0 };
	struct addrinfo* server;
	struct addrinfo* p;

	hints.ai_family = IP_FAMILY;
	hints.ai_socktype = SOCK_STREAM;

	if ((rc = getaddrinfo(host, PORT, &hints, &server)))
	{
		fprintf(stderr, "client: getaddrinfo: %s\n", gai_strerror(rc));
		freeaddrinfo(server);
		exit(3);
	}

	for (p = server; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL)
	{
		fprintf(stderr, "client: failed to connect\n");
		freeaddrinfo(server);
		exit(4);
	}

	freeaddrinfo(server);

	return sockfd;
}

WINDOW* newWindow(int y, int x, int height, int width, int color)
{
	WINDOW* win;

	y++;
	height--;
	x += 2;
	width -= 2;

	win = newwin(height, width, y, x);
	scrollok(win, TRUE);
	keypad(win, TRUE);
	printBox(win, color);

	return win;
}

void printBox(WINDOW* win, int color)
{
	int y, x, h, w;

	getbegyx(win, y, x);
	getmaxyx(win, h, w);

	y--;
	h++;
	x -= 2;
	w += 3;

	attron(color);

	mvaddch(y, x, ACS_ULCORNER);
	mvaddch(y, x + w, ACS_URCORNER);
	mvaddch(y + h, x, ACS_LLCORNER);
	mvaddch(y + h, x + w, ACS_LRCORNER);
	mvhline(y, x + 1, ACS_HLINE, w - 1);
	mvhline(y + h, x + 1, ACS_HLINE, w - 1);
	mvvline(y + 1, x, ACS_VLINE, h - 1);
	mvvline(y + 1, x + w, ACS_VLINE, h - 1);

	attroff(color);
	refresh();
}
