#include "makeSocket.h"
using namespace std;

const int BUFSIZE = 1024;

int MakeSocket(const char *host, int port) {
	int s; 			
	int len;	
	struct sockaddr_in sa; 
	struct hostent *hp;
	struct servent *sp;
	int portnum;	
	int ret;

	hp = gethostbyname(host);
	if (hp == 0) {
		perror("Bad hostname lookup");
		exit(1);
	}
	bcopy((char *)hp->h_addr, (char *)&sa.sin_addr, hp->h_length);
	sa.sin_family = hp->h_addrtype;
	sa.sin_port = htons(port);
	s = socket(hp->h_addrtype, SOCK_STREAM, 0);
	if (s == -1) {
		perror("Could not make socket");
		exit(1);
	}
	ret = connect(s, (struct sockaddr *)&sa, sizeof(sa));
	if (ret == -1) {
    perror("Could not connect");
		exit(1);
  }
	return s;
}
