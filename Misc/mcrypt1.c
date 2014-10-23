
/*
Test code for the mcrypt encryption library. Could replace char shift in sql_login_ui1.c.

Started with this code
  https://gist.github.com/bricef/2436364

Get the dev library
  apt-get install libmcrypt-dev

Compile with
  gcc -Wall mcrypt1.c -o mcrypt1 -lmcrypt

Run with
  Valgrind ./mcrypt1

C. Eric Cashon
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mcrypt.h> 

static int allocate_buffer_block(char **buffer, int length);
static int encrypt_string(void *buffer, int buffer_len, void *IV, void *key, int key_len);
static int decrypt_string(void *buffer, int buffer_len, void *IV, void *key, int key_len);

int main()
 {
  int i=0;
  //A string to encrypt.
  char string[]="A string to encrypt of some uncertain size."; 
  char *buffer=NULL;
  char IV[]="AAAAAAAAAAAAAAAA";
  char key[]="0123456789abcdef";
  int keysize=16; 
  int buffer_len=0;
  int length=strlen(string);

  //Check for a minimum string length.
  if(length>3)
    {
      //Get a block sized buffer to put the string in and return it's length.
      buffer_len=allocate_buffer_block(&buffer, length);
    }

  if(buffer_len>0)
    {
      strcpy(buffer, string); 
      printf("Start String\n");
      printf("  %s\n", buffer);
      encrypt_string(buffer, buffer_len, IV, key, keysize);
      printf("Encrypted String\n  ");
      for(i=0;i<strlen(buffer);i++)
         {
           printf("%i ", (int)buffer[i]);
         }
      printf("\n");
      decrypt_string(buffer, buffer_len, IV, key, keysize);
      printf("Decrypted String\n");
      printf("  %s\n", buffer);
      free(buffer);
    }

  return 0;
 } 
static int allocate_buffer_block(char **buffer, int length)
 {
  int i=0;
  int j=0;
   
  //Pad string for a block of 16.
  if(length<16) 
    {
      length=16;
      *buffer=(char*)malloc((length+1) * sizeof(char));
      if(*buffer==NULL)
        {
          printf("Malloc Error\n");
          return 0;
        }
      memset(*buffer, '\0', length+1);
    }
  else
    {
      i=length/16;
      j=length%16;
      if(j>0)
        {
          length=i*16+16;
          *buffer=(char*)malloc((length+1) * sizeof(char));
          if(*buffer==NULL)
            {
              printf("Malloc Error\n");
              return 0;
            }
          memset(*buffer, '\0', length+1);
        }
      else
        {
          length=i*16;
          *buffer=(char*)malloc((length+1) * sizeof(char));
          if(*buffer==NULL)
            {
              printf("Malloc Error\n");
              return 0;
            }
          memset(*buffer, '\0', length+1);
        }
    }
  
  return length;
 }
int encrypt_string(void *buffer, int buffer_len, void *IV, void *key, int key_len)
 {
   int i=0;
   MCRYPT td = mcrypt_module_open("twofish", NULL, "cfb", NULL);
   //MCRYPT td = mcrypt_module_open("rijndael-128", NULL, "cbc", NULL);
   int blocksize = mcrypt_enc_get_block_size(td);
   if(buffer_len % blocksize != 0)
     {
       printf("Blocksize Error %i %i\n", buffer_len, blocksize);
       return 1;
     }
   i=mcrypt_generic_init(td, key, key_len, IV);
   if(i<0)
     {
       mcrypt_perror(i);
       return 1;
     }
   mcrypt_generic(td, buffer, buffer_len);
   mcrypt_generic_deinit(td);
   mcrypt_module_close(td);
   return 0;
 } 
int decrypt_string(void *buffer, int buffer_len, void *IV, void *key, int key_len)
 {
   int i=0;
   MCRYPT td = mcrypt_module_open("twofish", NULL, "cfb", NULL);
   //MCRYPT td = mcrypt_module_open("rijndael-128", NULL, "cbc", NULL);
   int blocksize = mcrypt_enc_get_block_size(td);
   if(buffer_len % blocksize != 0)
     {
       printf("Blocksize Error %i %i\n", buffer_len, blocksize);
       return 1;
     }
   i=mcrypt_generic_init(td, key, key_len, IV);
   if(i<0)
     {
       mcrypt_perror(i);
       return 1;
     }
   mdecrypt_generic(td, buffer, buffer_len);
   mcrypt_generic_deinit(td);
   mcrypt_module_close(td);
   return 0;
 } 

