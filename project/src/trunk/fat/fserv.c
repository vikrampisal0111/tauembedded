#include "fserv.h"
#include "debug.h"
// Simple html for directory listing from Apache2.2 server.
// TODO: generate more decorated / informative documents.
static char html_head[] = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\"> \
<html> \
 <head> \
  <title>Index of /</title> \
 </head> \
 <body>\n";

static char html_dir[] = "<h1>Index of %s</h1>\n";

static char html_elem[] = "<ul><li><a href=\"%s\"> %s</a></li></ul>\n";

static char html_foot[] = "</body></html>\n";


FATFS fsdat; // Global file system container.

FRESULT fsInit()
{
   int r = SPI_Init();

   if (r) return FR_NOT_READY;
   return f_mount(0, &fsdat);
}

char fspath[MAX_PATH_LEN];

void constructFsPath(const char* path)
{
   // Construct file system path for the object.
   sprintf(fspath, "0:%s", path);
}


FRESULT fsGetElementInfo(const char* path, fsElemType* elemType, WORD* byteSize)
{
   FRESULT fsres;
   FILINFO inf;
   fsElemType type;
   WORD bytes;
   
   // Check if root dir.   
   if (!strcmp(path, "/")) {
      pmesg(MSG_DEBUG, "fserv: root directory info queried\n");
      if (elemType != NULL)
         *elemType = FSERV_DIR;
      return FR_OK;
   }

   constructFsPath(path);

   pmesg(MSG_DEBUG, "fserv: Full path = %s\n", fspath);

   // Get element type.
   fsres = f_stat(fspath, &inf);

   pmesg(MSG_DEBUG, "fserv: file stat retval = %d\n", fsres);
   pmesg(MSG_DEBUG, "fserv: stats size=%d, attrib=%x\n", inf.fsize, inf.fattrib);

   if (FR_OK != fsres) 
   {
      if ((FR_NO_FILE != fsres) && (FR_NO_PATH != fsres))
      {
         pmesg(MSG_DEBUG, "fserv: invalid path err! \n");
         return fsres;
      }
      type = FSERV_NONEXSIT;
   }

   // Otherwise, this means object exist and we can query the attrib info.
   if (inf.fattrib & 0x10) 
   {
      type = FSERV_DIR;
      //TODO: size for directory can be irrelevant, maybe need to check size in # of files.
      bytes = inf.fsize;
   }
   else
   {
      type = FSERV_FILE;
      bytes = inf.fsize;
   }

   if (elemType != NULL)
      *elemType = type;
   if (byteSize != NULL)
      *byteSize = bytes;

   return fsres;

}

/* Directory Content HTML consturction.
   in: directory object, directory path string, buffer to hold the data
   out: directory content html document will be written to buffer container
   retval: operaiton status
*/
FRESULT fsDirContentHtml(DIR *dirObj, const char* dirpath, char* dataBuff)
{
   int i = 0;
   FRESULT fsres = FR_OK;
   FILINFO inf;
   // Insert html header.
   sprintf(dataBuff, html_head);

   // Insert current directory string.
   sprintf(dataBuff+strlen(dataBuff), html_dir, dirpath);

   do
   {
      i = dirObj->index;
      fsres = f_readdir(dirObj, &inf);      
      if (i != dirObj->index) {
         sprintf(dataBuff+strlen(dataBuff), html_elem, inf.fname, inf.fname);
      }

   } while (i != dirObj->index);

   // Insert html footer.
   sprintf(dataBuff+strlen(dataBuff), html_foot);
  


   return FR_OK;


}


FRESULT fsGetElementData(const char* path, char* dataBuff)
{
   FRESULT fsres = FR_OK;
   FIL file;   
   DIR dir;
   WORD bytesRead, byteSize;
   fsElemType type;

   fsres = fsGetElementInfo(path, &type, &byteSize);

   pmesg(MSG_DEBUG, "get element info result = %d\n", fsres);
   if (fsres) return fsres;

   switch (type)
   {
   case FSERV_FILE:      

      fsres = f_open(&file, fspath, FA_READ);
      if (fsres != FR_OK) 
      {
         return fsres;
      }      
   
      fsres = f_read(&file, dataBuff, byteSize, &bytesRead);

      if (byteSize != bytesRead)
      {
         return FR_RW_ERROR; 
      }
      if (fsres) return fsres;

      fsres = f_close(&file);

      if (fsres) return fsres;
      
      break;
   case FSERV_DIR:
      
      fsres = f_opendir(&dir, fspath);

      if (fsres) return fsres;
      
      fsres = fsDirContentHtml(&dir, path, dataBuff);

      if (fsres) return fsres;

      break;      
   case FSERV_NONEXSIT:
   default:
      return FR_INVALID_OBJECT;
   }



   return fsres;

}




// End of file.

