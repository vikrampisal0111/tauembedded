
#include <stdint.h>
#include <io.h>

#include "fat/ff.h"
#include "spi1.h"
#include "string.h"
// Check max possible value for dir (<=3)
#define TST_NUM_DIR 4
#define TST_NUM_FILE 5


int printDirContent(char* dirname)
{

int err;

// List dir.
DIR dirEntry;
FILINFO nxtEntry;
err = f_opendir(&dirEntry, dirname);

if (err != FR_OK)
{
 printf("open dir error = %d\n",err);
}
else
{
printf("\ndir list %s:\n--------------\n",dirname);
int i = 0;
  do
  {

   i = dirEntry.index;
   f_readdir(&dirEntry, &nxtEntry); 

   if (i != dirEntry.index)
	   printf("%s : %d : %x : %c\n", nxtEntry.fname, nxtEntry.fsize, nxtEntry.fattrib, (nxtEntry.fattrib & 0x10)?('D'):('F'));

  } while(i!=dirEntry.index);
}

printf("-----end list-------\n");

}

int main(void) { 
  int d, i, err;
  int testok = 1;
  FATFS fsdat;
  FIL file;
  uart0Init();  
  SPI_Init(); 
   printf("Fat load result:\n");
   printf("%d\n",f_mount(0, &fsdat));
char cur_file[20] = "0:/tst1.txt";
char cur_dir[20] = "0:/dir1";
for (d = 0; d < TST_NUM_DIR; d++)
{
  if (d > 0)
  {
    // Create directory.
    cur_dir[6] = '0' + (char) d;

    printf("***create dir %s",cur_dir);
    err = f_mkdir(cur_dir);

    if (err != FR_OK && err != FR_EXIST)
    {
    	printf("error creating directory, err = %d\n",err);
	return err;
    } 
    
    strcpy(cur_file, cur_dir);
    strcpy(cur_file+strlen(cur_file), "/tst1.txt"); 
   
  }


  for (i = 0; i < TST_NUM_FILE; i++)
  {
     printf("\nFat FILE %d CREATE result:\n", i);

     cur_file[(d==0)?6:11] = '0'+(char)i+1;
     printf("***open file:%s\n", cur_file);
     err = f_open(&file, cur_file, FA_CREATE_ALWAYS | FA_OPEN_ALWAYS| FA_WRITE);

     char * str = "hello world!";
     WORD bw;
     if (err != FR_OK && err != FR_EXIST) 
     {
        printf("error creating file\n");
        return err;
     }
  
     err = f_write(&file, str, strlen(str)+1,&bw);

     printf("Bytes written %d\n", bw);
     if (err != FR_OK)
     { 
	printf("error writing file %d\n", err);
	return err;
     }

     err = f_sync(&file);

     if (err != FR_OK)
     {
        printf("error flush file %d\n", err);
        return err;
     }

     err =  f_close(&file);
     if (err != FR_OK)
     {
	printf("error closing file %d\n",err);
	return err;
     }

  }   
}

printDirContent("0:/");
printDirContent("0:/dir1");
printDirContent("0:/dir2");
printDirContent("0:/dir3");

// Test file delete / directory delete
printf("test delete of 0:/dir1/tst1.txt\n");

err = f_unlink("0:/dir1/tst1.txt");

if (err != FR_OK)
{
	printf("test delete of 0:/dir2\n");
	return;
}
err = f_unlink("0:/dir2/*.txt");

if (err != FR_OK)
{
	printf("error delete %d\n", err);
	return err;
}
// Test read file content.
err = f_open(&file, "0:/tst1.txt",FA_READ);

printf("test read from 0:/dir1/tst1.txt\n");

if (err != FR_OK)
{
  printf("file open err = %d\n", err);
  return err;
}
char data[50];
WORD br;
err = f_read(&file, data, 13, &br);

if (err != FR_OK)
{
	printf("read file error %d\n", err);
	return err;
}

printf("bytes read %d\n", br);

printf("data read %s\n", data);

err = f_close(&file);

if (err != FR_OK)
{
	printf("close err = %d\n",err);
}

}
