
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <ctype.h>
#include <sstream>
#include <iostream>
#include <string>

#include "AppPush.h"


int main(int argc, char* argv[])
{
    
    AppPush theApp;
	theApp.setConnectEvent(&theApp);
	theApp.runAll(argc,argv);
    return 0;
    
}

