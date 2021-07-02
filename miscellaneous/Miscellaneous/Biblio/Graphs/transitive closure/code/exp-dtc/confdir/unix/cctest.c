#include <iostream.h>

main()
{
  char* cc = "CC";

#if defined(__SUNPRO_CC)
  cc = "sunpro";
#endif

#if defined(hpux)
  cc = "hpux";
#endif

#if defined(__CLCC__)
  cc = "centerline";
#endif

#if defined(mips) 
#if defined(_COMPILER_VERSION)
  cc = "mipspro";
#else
  cc = "mips";
#endif
#endif

  cout << cc << endl;

  return 0;
}
