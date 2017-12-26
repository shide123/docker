
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <ctype.h>
#include <sstream>
#include <iostream>
#include <string>

#include "AppDelegate.h"
int main(int argc, char* argv[])
{
	AppDelegate theApp;
	theApp.setConnectEvent(&theApp);
	theApp.runAll(argc,argv);
    return 0;
}

