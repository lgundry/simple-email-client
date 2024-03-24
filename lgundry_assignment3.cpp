// Logan Gundry
#include <iostream>	
#include <assert.h>
#include <ctime>
#include <stdio.h>	
#include <string.h> 
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <fstream>
#include <cstdint>

using namespace std;

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

string getMailServer(string domain){
    char buf[128];
    string ans = "";
    string delimiter = "mail exchanger = ";

    // I only really need one mx server, why bother getting all of them
    string command = "nslookup -type=mx " + domain + " | head -5 | tail +5";

    // Partially GitHub CoPilot
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        cout << "ns-lookup failed." << endl;
        return 0;
    }
    if (fgets(buf, sizeof buf, pipe) == NULL) {
        perror("No associated mx server");
        return 0;
    }
    pclose(pipe);
    // end CoPilot assistance

    // add chars onto ans until end of buf
    for (int i = 0; i < sizeof buf; i++) {
        if(buf[i] != '\n')
            ans += buf[i];
        else
            break;
    }

    int addressLocation = ans.find(delimiter) + delimiter.length();

    
    //https://cplusplus.com/reference/string/string/erase/
    if (ans[addressLocation + 2] == ' ')
        ans.erase(0, addressLocation + 3);
    else
        ans.erase(0, addressLocation + 2);

    return ans;
}

// use struct tm from ctime lib to generate the date and time in a format that works for smtp server dialogue
// https://cplusplus.com/reference/ctime/tm/
string getDate(){

  time_t now = time(0);

  tm *date = localtime(&now);
  string year, month, wday, mday, time;
  year = to_string(1990 + date->tm_year);

  // get month in english - using a switch
  switch (date->tm_mon + 1){
    case 1:
      month = "Jan";
      break;
    case 2:
      month = "Feb";
      break;
    case 3:
      month = "Mar";
      break;
    case 4:
      month = "Apr";
      break;
    case 5:
      month = "May";
      break;
    case 6:
      month = "Jun";
      break;
    case 7:
      month = "Jul";
      break;
    case 8:
      month = "Aug";
      break;
    case 9:
      month = "Sep";
      break;
    case 10:
      month = "Oct";
      break;
    case 11:
      month = "Nov";
      break;
    case 12:
      month = "Dec";
      break;
  }
  
  // get day in english - using switch
  switch(date->tm_wday + 1){
    case 1:
      wday = "SUN";
      break;
    case 2:
      wday = "MON";
      break;
    case 3:
      wday = "TUE";
      break;
    case 4:
      wday = "WED";
      break;
    case 5:
      wday = "THU";
      break;
    case 6:
      wday = "FRI";
      break;
    case 7:
      wday = "SAT";
      break;
  }

  mday = to_string(date->tm_mday);
  time = to_string(date->tm_hour) + ":" + to_string(date->tm_min) + ":" + to_string(date->tm_sec);

  return wday + ", " + mday + " " + month + " " + year + " " + time;

}

// write to smtp server - same syntax as write()
int writeToSMTP(int &socket, const char *request, int size){
  // cout << request;
  int len = write(socket, request, size);
  if (len == -1){
    perror("write failed");
    exit(1);
  }
  return len;
}

// read from smtp server - same syntax as read()
int readFromSMTP(int &socket, char *buf, int nbytes){
  int len = read(socket, buf, nbytes);
  if (len == -1){
    perror("read failed");
    exit(1);
  }

  // if read contains error information, print it and exit the program
  if ((buf[0] - '0') != 2 && (buf[0] - '0') != 3){
    perror(buf);
    int temp = writeToSMTP(socket, "QUIT", 5); //don't leave server hangin
    exit(1);
  }
  return len;
}

// Break string into 2 pieces at first delimiter. Returns the second half, and cuts the second half off of the first
string breakdown(char delim, string &toBreak) {
  int delimLocation = toBreak.find(delim);

  //https://cplusplus.com/reference/string/string/substr/
  string secondHalf = toBreak.substr(delimLocation + 1);
  toBreak.erase(delimLocation, toBreak.length());

  return secondHalf;
}

int main(int argc, char *argv[]) {

  // Vars
  string sender, senderHostName, dest, destHostName;
  const int BUFSIZE = 1024;
  const int portNum = 25;
  char buf[BUFSIZE];
  int len, len2;
  const string messageID = "Message-ID:<CAEb5kgpPWSWWqL0Kj=Nui2NbTcF+_tB6CKXfAQzdZBpK6wzp=w@mail.gmail.com>\r\n";

  // Begin Dialogue------------

  cout << "To: "; //johndoe@gmail.com
  getline(cin, dest);
  destHostName = breakdown('@', dest);

  cout << "From: "; //jbidin@whitehouse.gov
  getline(cin, sender);
  senderHostName = breakdown('@', sender);

  int sock = MakeSocket(getMailServer(destHostName).c_str(), portNum); 
  if (sock == -1)
  {
    perror("Bad connection");
    exit(1);
  }
  len2 = readFromSMTP(sock, buf, BUFSIZE);

  // initiate dialogue with server
  string handshake = "HELO " + senderHostName + "\r\n";
  len = writeToSMTP(sock, handshake.c_str(), handshake.length());
  len2 = readFromSMTP(sock, buf, BUFSIZE);

  // get username of sender (does not include)
  string mailFrom = "MAIL FROM:<" + sender + "@" + senderHostName + ">\r\n";
  len = writeToSMTP(sock, mailFrom.c_str(), mailFrom.length());
  len2 = readFromSMTP(sock, buf, BUFSIZE);

  // RCPT TO
  string destAddress = "RCPT TO:<" + dest + "@" + destHostName + ">\r\n";
  len = writeToSMTP(sock, destAddress.c_str(), destAddress.length());
  len2 = readFromSMTP(sock, buf, BUFSIZE);

  // Start data
  len = writeToSMTP(sock, "data\r\n", 6);
  len2 = readFromSMTP(sock, buf, BUFSIZE);

  // Date
  string date = "DATE:" + getDate() + "\r\n";
  len = writeToSMTP(sock, date.c_str(), date.length());

  // Message-ID
  len = writeToSMTP(sock, messageID.c_str(), messageID.length());

  // Subject:
  string subject = "";
  cout << "subject: ";
  getline(cin, subject);
  subject = "Subject: " + subject + "\r\n";
  len = writeToSMTP(sock, subject.c_str(), subject.length());

  // To:
  string to = "To: " + dest + "@" + destHostName + "\r\n";
  len = writeToSMTP(sock, to.c_str(), to.length());
  
  // From:
  string from = "From: " + sender + "@" + senderHostName + "\r\n";
  len = writeToSMTP(sock, from.c_str(), from.length());

  // Message:
  cout << "Enter a message. End that message with a '.' on a line by itself:" << endl;
  string thisLine = " ";
  string endMessage = ".\r\n";
  while (endMessage != thisLine)
  {
    getline(cin, thisLine);
    thisLine += "\r\n";
    writeToSMTP(sock, thisLine.c_str(), thisLine.length());
  }
  readFromSMTP(sock, buf, BUFSIZE);

  // Quit
  writeToSMTP(sock, "QUIT\r\n", 7);
  readFromSMTP(sock, buf, BUFSIZE);

  // End Dialogue-----------------

  // If anything fails, the program will not execute up to this point, so as long as the message
  // doesn't get blocked by a spam filter, this is true
  cout << "Message sent" << endl;
  return 0;
}
