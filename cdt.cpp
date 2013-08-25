// A simple program that generates spacetime
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include "CDTConfig.h"

using namespace std;

int main (int argc, char *argv[])
{
  if (argc < 2)
    {
    fprintf(stdout,"%s Version %d.%d\n",
            argv[0],
            CDT_VERSION_MAJOR,
            CDT_VERSION_MINOR);
    fprintf(stdout,"Usage: %s number\n",argv[0]);
    return 1;
    }

  cout << "Sorry, I don't do anything yet" << endl;
  return 0;
}
