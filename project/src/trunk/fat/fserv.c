#include "fserv.h"
#include "debug.h"
//#include "lcd.h"

#define DIR_LST_SIZE	(4 * 1024)

#define IP_COOKIE "0:/ip.bin"

// Simple html for directory listing from Apache2.2 server.
// TODO: generate more decorated / informative documents.
static char html_head[] = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\"> \
<html> \
    <head> \
	<title>Index of %s</title> \
    </head> \
    <body>\n";

static char html_dir[] = "<h1>Index of %s</h1>\n";

//static char html_elem[] = "<ul><li><a href=\"%s%s%s\"> %s</a></li></ul>\n";

static char dir_icon[] = "/icons/dir.gif";
static char file_icon[] = "/icons/file.gif";

// [icon] [link] [link_name] [size]
static char html_elem[] = "<tr> \
    <td valign=\"top\"><img src=\"%s\"></td> \
    <td><a href=\"%s%s%s\">%s</a></td>\n";

static char html_elem_size[] = "<td align=\"right\">%d</td></tr>";
static char html_elem_nosize[] = "<td align=\"right\">-</td></tr>";

static char html_elem_head[] = "<table> \
<tr> \
    <th>&nbsp;</th> \
    <th>Name</th> \
    <th>Size</th> \
</tr>\n";

static char html_elem_foot[] = "<tr><th colspan=\"3\"><hr></th></tr> \
</table> \
<address>LPC2148 NAS (uIP/1.0) Server at %s Port 80</address>\n";

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


FRESULT fsGetElementInfo(const char* path, fsElemType* elemType, DWORD* byteSize)
{
   FRESULT fsres;
   FILINFO inf;
   fsElemType type;
   DWORD bytes;
   
   // Check if root dir.   
   if (!strcmp(path, "/")) {
      constructFsPath("/");
      pmesg(MSG_DEBUG, "fserv: root directory info queried\n");
      if (elemType != NULL)
         *elemType = FSERV_DIR;
      if (byteSize != NULL)
         *byteSize = DIR_LST_SIZE; // Constant buffer size.
      return FR_OK;
   }

    constructFsPath(path);

    pmesg(MSG_INFO, "fserv: Full path = %s\n", fspath);

    // Get element type.
    fsres = f_stat(fspath, &inf);

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
	bytes = DIR_LST_SIZE; 
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
FRESULT fsDirContentHtml(DIR *dirObj, const char* dirpath, char* dataBuff, int bufflen)
{
    int i = 0;
    FRESULT fsres = FR_OK;
    FILINFO inf;
    // Insert html header.
    snprintf(dataBuff, bufflen, html_head, dirpath);

    // Insert current directory string.
    snprintf(dataBuff + strlen(dataBuff), bufflen - strlen(dataBuff), html_dir, dirpath);

    // Add table start and headers
    snprintf(dataBuff + strlen(dataBuff), bufflen - strlen(dataBuff), html_elem_head);

    do
    {
	char *separator = "\0";
	i = dirObj->index;
	fsres = f_readdir(dirObj, &inf);
	if(i == dirObj->index) {
	    break;
	}
	if (dirpath[strlen(dirpath)] != '/') //avoid double slash 
	    separator = "/";
	if (!strcmp(dirpath,"/"))
	{
	    separator = "\0";
	    dirpath ="\0";
	}
	if (i != dirObj->index) {
	    int isdir = (inf.fattrib & AM_DIR);
	    char *icon_path = (isdir) ? dir_icon : file_icon; 
	    snprintf(dataBuff + strlen(dataBuff), bufflen - strlen(dataBuff), 
		    html_elem, 
		    icon_path, // icon path
		    dirpath, separator, inf.fname, // link path
		    inf.fname); // link name

	    if(!isdir) {
		snprintf(dataBuff + strlen(dataBuff), bufflen - strlen(dataBuff),
			html_elem_size,
			inf.fsize); // size
	    }
	    else {
		strncat(dataBuff + strlen(dataBuff), html_elem_nosize, bufflen - strlen(dataBuff));
	    }
	}

    } while (i != dirObj->index);

    // Add table footer
    snprintf(dataBuff + strlen(dataBuff), bufflen - strlen(dataBuff), html_elem_foot);

    // Insert html footer.
    snprintf(dataBuff + strlen(dataBuff), bufflen - strlen(dataBuff), html_foot);

    return FR_OK;
}

char dir_list_buffer[DIR_LST_SIZE];

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

    //lcdClearScreen();
    //lcdPrintString(path);
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

		pmesg(MSG_DEBUG, "start building html\n");
		fsres = fsDirContentHtml(&dir, path, dir_list_buffer, sizeof(dir_list_buffer));

		if (fsres) return fsres;
		pmesg(MSG_DEBUG, "built html: \n\n|%s|\n\n", dir_list_buffer);
	    }
	    pmesg(MSG_DEBUG, "\nlist offset = %d, bytes to read = %d\n", offset, bytesToRead);
	    //finally copy partial list info into buffer.
	    memcpy(dataBuff, dir_list_buffer + offset, bytesToRead);

	    break;      
	case FSERV_NONEXSIT:
	default:
	    return FR_INVALID_OBJECT;
    }



    return fsres;

}

void fsSetIp(const unsigned char* pIp)
{
   FRESULT fsres;
   FIL file; 
   WORD bytesWritten;




   fsres = f_open(&file, IP_COOKIE, FA_CREATE_ALWAYS | FA_WRITE);
   pmesg(MSG_INFO, "open ip cookie : ___ ");
   if (fsres != FR_OK) return;

// write ip 4 bytes.
   fsres = f_write(&file, pIp, 4, &bytesWritten);

   if (fsres != FR_OK) return;
   pmesg(MSG_INFO, "written %d bytes to ip cookie\n", bytesWritten);

   fsres = f_close(&file);
   
}

void fsGetIp(unsigned char* pIp)
{
   FRESULT fsres;
   FIL file;
   FILINFO inf;
   WORD bytesRead;

  //default IP is zero in case of error.
   memset(pIp, 0, 4);

   fsres = f_stat(IP_COOKIE, &inf);

   if ((inf.fsize < 4) || (fsres != FR_OK))
   	return;

   fsres = f_open(&file, IP_COOKIE, FA_READ);
   pmesg(MSG_INFO, "open ip cookie : ___ ");
   if (fsres != FR_OK) return;

// write ip 4 bytes.
   fsres = f_read(&file, pIp, 4, &bytesRead);

   if (fsres != FR_OK) return;
   pmesg(MSG_INFO, "read %d bytes from ip cookie\n", bytesRead);

   fsres = f_close(&file);

   return; 
}




// End of file.

