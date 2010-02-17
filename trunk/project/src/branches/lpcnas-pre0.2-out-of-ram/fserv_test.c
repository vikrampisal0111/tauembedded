
#include <stdint.h>
#include <io.h>

#include "debug.h"

#include "fat/ff.h"
#include "spi1.h"
#include "string.h"
#include "fat/fserv.h"

DEFINE_pmesg_level(MSG_DEBUG);

static char buf[1024]; // buffer for docuemnts.

int testDir(const char *path)
{
   FRESULT frs;
   fsElemType type;
   WORD size;
   pmesg(MSG_INFO,"testing element '%s':\n", path);
   frs = fsGetElementInfo(path, &type, &size);
   if (frs) {
      pmesg(MSG_INFO,"frs = %d\n",frs);
      return -1;
   }
   if (type != FSERV_DIR) {
      pmesg(MSG_INFO,"error '%s' is not recognized as a directory...\n", path);     
      return -1;
   }
   else {
      pmesg(MSG_INFO,"reading directory content:\n");
      frs = fsGetElementData(path, buf);
      if (frs) {
         pmesg(MSG_INFO,"frs = %d\n",frs);
         return -1;
      }
      pmesg(MSG_INFO,buf);
   }
   return 0;
}

int testFile(const char *path)
{
   FRESULT frs;
   fsElemType type;
   WORD size;
   pmesg(MSG_INFO,"testing element '%s':\n", path);
   frs = fsGetElementInfo(path, &type, &size);
   if (frs) {
      pmesg(MSG_INFO,"frs = %d\n",frs);
      return -1;
   }
   if (type != FSERV_FILE) {
      pmesg(MSG_INFO,"error '%s' is not recognized as a file...\n", path);     
      return -1;
   }
   else {
      pmesg(MSG_INFO, "file size = %d\n", size);
      pmesg(MSG_INFO,"reading file content:\n");
      frs = fsGetElementData(path, buf);
      if (frs) {
         pmesg(MSG_INFO,"frs = %d\n",frs);
         return -1;
      }
      pmesg(MSG_INFO,buf);
   }

   return 0;
}

int main(void) { 
  FRESULT frs;

//  uart0Init();
  fopen("uart0", "w");

  frs = fsInit();

  if (!frs)
  pmesg(MSG_INFO,"init OK res = %d\n", frs);

  if (testDir("/")) return -1;
  if (testDir("/dir1")) return -1;
  if (testDir("/dir2")) return -1;
  if (testFile("/tst1.txt")) return -1;
  if (testFile("/tst2.txt")) return -1;
  if (testFile("dir1/tst3.txt")) return -1;
  if (testFile("dir2/tst2.txt")) return -1;
  pmesg(MSG_INFO,"   *** Test completed successfully!\n");
  //fclose("uart0");

  return 0;


}

