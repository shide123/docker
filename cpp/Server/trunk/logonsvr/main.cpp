
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <ctype.h>
#include <sstream>
#include <iostream>
#include <string>

#include "LogonSvrApp.h"
int main(int argc, char* argv[])
{
	CLogonSvrApp theApp;
	theApp.setConnectEvent(&theApp);
	theApp.runAll(argc,argv);
    return 0;
}

