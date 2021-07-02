#include <iostream.h>

int main()
{
  double x = 1;
  long *px = (long *) &x;

  if (sizeof(long) == sizeof(double))
     cout << "DFLAGS = -DWORD_LENGTH_64\n";
  else
     if (!(*px) && *(++px)) 
        cout <<   "DFLAGS = -DLITTLE_ENDIAN\n";
/*
     else
        if (*px && !(*(++px)))
            cout << "DFLAGS = -DBIG_ENDIAN\n";
*/

  return 0;
}
