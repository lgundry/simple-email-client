#include "makeSocket.h"
#include "socket.cpp"
#include <string>
#include <iostream>
using namespace std;

// If the user enters google.com, gmail.com, or nmu.edu, it will correct it, otherwise its up to the user
string getSMTP(string domain){
  if (domain == "gmail.com"){
    return "gmail-smtp-in.l.google.com";
  }
  else if (domain == "google.com"){
    return "smtp.google.com";
  }
  else if (domain == "nmu.edu"){
    return "ASPMX.L.GOOGLE.COM";
  }
  return domain;
}

int readFromSMTP(int &socket, char *buf, int nbytes){
  int len = read(socket, buf, nbytes);
  if (len == -1){
    perror("read failed");
    int temp = writeToSMTP(socket, "QUIT", 5); //don't leave server hangin
    exit(1);
  }
  if (buf[0] != 2 && buf[0] != 3){
    perror("bad request");
    int temp = writeToSMTP(socket, "QUIT", 5); //don't leave server hangin
    exit(1);
  }
  return len;
}

int writeToSMTP(int &socket, const char *request, int size){
  int len = write(socket, request, size);
  if (len == -1){
    perror("write failed");
    exit(1);
  }
  return len;
}

int main(int argc, char *argv[]) {

  // Vars
  string sender, senderHostName, recipient, server, message;
  const int BUFSIZE = 1024;
  char buf[BUFSIZE];
  int len = 1, len2 = 1;

  // Begin Dialogue with user------------

  // get recipient
  cout << "Who're we memeing today boss?> ";
  getline(cin, recipient);
  if(!recipient.find("@")){
    perror("ENTER RECIPIENT COMPLETE ADDRESS");
    exit(1);
  }

  // get smtp server and verify connection
  cout << "What host are we sending to?";
  getline(cin, server);
  server = getSMTP(server);
  int sock = MakeSocket(server.c_str(), 25); 
  if (sock == -1)
  {
    perror("Bad connection");
    exit(1);
  }
  len2 = readFromSMTP(sock, buf, BUFSIZE);

  // get sender address and initiate dialogue with server
  cout << "I'm feeling fraudulent. What hostname shall we pretend to be?> "; // domain.tld
  getline(cin, senderHostName);
  senderHostName = "HELO " + senderHostName + "\r\n";
  len = writeToSMTP(sock, senderHostName.c_str(), senderHostName.length());
  len2 = readFromSMTP(sock, buf, BUFSIZE);

  cout << "And who would you like to be?> ";
  getline(cin, sender);
  string mailFrom = "MAIL FROM:<" + sender + ">\r\n";
  len = writeToSMTP(sock, mailFrom.c_str(), mailFrom.length());
  len2 = readFromSMTP(sock, buf, BUFSIZE);

  cout << "Enter the message.  End that message with a '.' on a line by itself:" << endl;
  string thisLine = "";
  do
  {
    thisLine = "";
    getline(cin, thisLine);
    message += "\r\n" + thisLine;
  } while (thisLine != ".");

  len = writeToSMTP(sock, "<CAEb5kgpPWSWWqL0Kj=Nui2NbTcF+_tB6CKXfAQzdZBpK6wzp=w@mail.gmail.com>", senderAddress.length())

  char buf[BUFSIZE];
  int len2 = read(sock, buf, BUFSIZE-1);
  buf[len2] = 0;
  cout << buf;
}
