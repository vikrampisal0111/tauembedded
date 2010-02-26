#include "fserv.h"
#include "debug.h"
#include "lcd.h"

// Simple html for directory listing from Apache2.2 server.
// TODO: generate more decorated / informative documents.
static char html_head[] = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\"> \
<html> \
 <head> \
  <title>Index of /</title> \
 </head> \
 <body>\n";

static char html_dir[] = "<h1>Index of %s</h1>\n";

static char html_elem[] = "<ul><li><a href=\"%s%s%s\"> %s</a></li></ul>\n";

static char html_foot[] = "</body></html>\n";


FATFS fsdat; // Global file system container.

FRESULT fsInit()
{
   int r = SPI_Init();

   lcdInit(); // init lcd display.

   if (r) return FR_NOT_READY;
   return f_mount(0, &fsdat);
}

char fspath[MAX_PATH_LEN];

void constructFsPath(const char* path)
{
   // Construct file system path for the object.
   sprintf(fspath, "0:%s", path);
}


FRESULT fsGetElementInfo(const char* path, fsElemType* elemType, DWORD* byteSize)
{
   FRESULT fsres;
   FILINFO inf;
   fsElemType type;
   DWORD bytes;
   
   // Check if root dir.   
   if (!strcmp(path, "/")) {
      constructFsPath("/");
      pmesg(MSG_INFO, "fserv: root directory info queried\n");
      if (elemType != NULL)
         *elemType = FSERV_DIR;
      if (byteSize != NULL)
         *byteSize = 2048; // Constant buffer size.
      return FR_OK;
   }

   constructFsPath(path);

   pmesg(MSG_INFO, "fserv: Full path = %s\n", fspath);

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
      return FR_OK;
   }

   // Otherwise, this means object exist and we can query the attrib info.
   if (inf.fattrib & 0x10) 
   {
      type = FSERV_DIR;
      //TODO: size for directory can be irrelevant, maybe need to check size in # of files.
      bytes = 2048; // 19.2.10 - fixed 2KB size (as max buffer size)
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
      char *separator = "\0";
      i = dirObj->index;
      fsres = f_readdir(dirObj, &inf);      
      if (dirpath[strlen(dirpath)] != '/') //avoid double slash 
         separator = "/";
      if (!strcmp(dirpath,"/"))
      {
	 separator = "\0";
         dirpath ="\0";
      }
      if (i != dirObj->index) {
         sprintf(dataBuff+strlen(dataBuff), html_elem, dirpath, separator, inf.fname, inf.fname);
      }

   } while (i != dirObj->index);

   // Insert html footer.
   sprintf(dataBuff+strlen(dataBuff), html_foot);
  


   return FR_OK;


}

char dir_list_buffer[2*1024];

FRESULT fsGetElementData(const char* path, char* dataBuff, 
int offset, int bytesToRead)
{
   FRESULT fsres = FR_OK;
   FIL file;   
   DIR dir;
   DWORD bytesRead, byteSize;
   fsElemType type;

   fsres = fsGetElementInfo(path, &type, &byteSize);

   pmesg(MSG_DEBUG, "get element info result = %d\n", fsres);
   if (fsres) return fsres;

   lcdClearScreen();
   lcdPrintString(path);
   switch (type)
   {
   case FSERV_FILE:      
      fsres = f_open(&file, fspath, FA_READ);
      pmesg(MSG_INFO, "serving file\n");
      if (fsres != FR_OK) 
      {
         return fsres;
      }      

      fsres = f_lseek(&file, offset);

      if (fsres) return FR_RW_ERROR; 
   
      fsres = f_read(&file, dataBuff, bytesToRead, &bytesRead);

      if (byteSize != bytesRead)
      {
         return FR_RW_ERROR; 
      }
      if (fsres) return fsres;

      fsres = f_close(&file);

      if (fsres) return fsres;
      
      break;
   case FSERV_DIR:

      if (offset == 0)
      {      
        memset(dir_list_buffer, 0, sizeof(dir_list_buffer));
      	fsres = f_opendir(&dir, fspath);
        pmesg(MSG_INFO, "serving directory %s\n", fspath);

      	if (fsres) return fsres;
      
        printf("start build html\n");
      	fsres = fsDirContentHtml(&dir, path, dir_list_buffer);
        
      	if (fsres) return fsres;
        printf("build html: *** \n\n %s ***\n\n", dir_list_buffer);
      }
      printf("\nlist offset = %d, bytes to read = %d\n", offset, bytesToRead);
//finally copy partial list info into buffer.
      memcpy(dataBuff, dir_list_buffer + offset, bytesToRead);

      break;      
   case FSERV_NONEXSIT:
   default:
      return FR_INVALID_OBJECT;
   }



   return fsres;

}




// End of file.

