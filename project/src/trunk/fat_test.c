
#include <stdint.h>
#include <io.h>

#include "fat/ff.h"
#include "spi1.h"

// Check max possible value for dir (<=3)
#define TST_NUM_DIR 3
#define TST_NUM_FILE 5

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

    printf("create dir %s",cur_dir);
    err = f_mkdir(cur_dir);

    if (err != FR_OK && err != FR_EXIST)
    {
    	printf("error creating directory, err = %d\n",err);
	break;
    } 
    
    strcpy(cur_file, cur_dir);
    strcpy(cur_file+strlen(cur_file), "/tst1.txt"); 
   
  }


  for (i = 0; i < TST_NUM_FILE; i++)
  {
     printf("\nFat FILE %d CREATE result:\n", i);

     cur_file[(d==0)?6:11] = '0'+(char)i+1;
     printf("file:%s\n", cur_file);
     err = f_open(&file, cur_file, FA_CREATE_ALWAYS | FA_READ | FA_WRITE);

     char * str = "hello world!";
     DWORD bw;
     if (err != FR_OK && err != FR_EXIST) 
     {
        printf("error creating file\n");
        break;
     }
  
     f_write(&file, str, strlen(str),&bw);

     printf("\nFat FILE CLOSE result:\n");
     printf("%d\n",f_close(&file)); 
     printf("Bytes written %d\n", bw);

  }   
}

// List first dir.
DIR dirEntry;
FILINFO nxtEntry;
err = f_opendir(&dirEntry, "0:/dir1");

if (err != FR_OK)
{
 printf("open dir error = %d\n",err);
}
else
{
printf(" dir list 0:/dir1:\n--------------\n");
  for (i = 0; i < TST_NUM_FILE; i++)
  {
   f_readdir(&dirEntry, &nxtEntry); 

   printf("Filename %s\n", nxtEntry.fname);
    
  }
}








}
