#pragma once
#include <iostream>	
#include <assert.h>	
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

int MakeSocket(const char *host, int port);
