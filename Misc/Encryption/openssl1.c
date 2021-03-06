
/*
Test code for the OpenSSL encryption library using AES 256. For a comparison with the mcrypt code. 

Started with this code
  http://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption

Get the dev library
  sudo apt-get install libssl-dev

Compile with
  gcc -Wall openssl1.c -o openssl1 -lcrypto

Info about valgrind with OpenSSL
  https://www.openssl.org/support/faq.html

Run with
  Valgrind ./openssl1

C. Eric Cashon
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<openssl/conf.h>
#include<openssl/evp.h>
#include<openssl/err.h>

static int allocate_buffer_block(char **buffer, int length, int block_size);
static void handleErrors(void);
static int encrypt_string(const char *string, int string_len, char *key, char *iv, char *buffer1);
static int decrypt_string(char *buffer1, int encrypt_len, char *key, char *iv, char *buffer2);

int main()
 {
  int i=0;
  const char password[]="password";
  //For hashing a key from the password.
  unsigned char salt_value[] = {'s','a','l','t'};
  int iterations=10;
  unsigned char key[32];
  int key_size=32;
  //A string to encrypt.
  const char string[]="A string to encrypt of some uncertain length."; 
  //Buffers for encryption and decryption.
  char *buffer1=NULL;
  char *buffer2=NULL; 
  int buffer_len1=0;
  int length1=strlen(string);
  int length2=0;
  /*
    Store the IV in the database and retrieve to decrypt. The IV changes the encryption output
    for the same input and should be randomly generated. Just use a constant here.
  */
  char IV[]="01234567890123456";

  //Initialize OpenSSL.
  ERR_load_crypto_strings();
  OpenSSL_add_all_algorithms();
  OPENSSL_config(NULL);

  //Hash a key from the password.
  PKCS5_PBKDF2_HMAC_SHA1(password, strlen(password), salt_value, sizeof(salt_value), iterations, key_size, key);
  printf("Hash Key\n  ");
  for(i=0;i<key_size;i++) { printf("%02x", key[i]); }
  printf("\n");

  //Check for a minimum string length.
  if(length1>3)
    {
      //Get a block sized buffer to put the string in and return it's length.
      buffer_len1=allocate_buffer_block(&buffer1, length1, 32);
      //Second buffer same length as first. Can use just one buffer.
      allocate_buffer_block(&buffer2, length1, 32);
    }

  if(buffer_len1>0)
    { 
      printf("Start String\n");
      printf("  %s\n", string); 
      length2=encrypt_string(string, length1, IV, (char *)key, buffer1);
      //Note the difference in string length and the encrypted length.
      printf("String Length %i Buffer Length %i Encrypted Length %i\n", length1, buffer_len1, length2); 
      printf("Encrypted String  \n  ");
      for(i=0;i<length2;i++)
         {
           printf("%i ", (int)buffer1[i]);
         }
      printf("\n");
      decrypt_string(buffer1, length2, IV, (char *)key, buffer2);
      //The encryption overwrites extra bytes at the end of the buffer. Add a null character.
      buffer2[length1]='\0';
      printf("Decrypted String\n");
      printf("  %s\n", buffer2);
      free(buffer1);
      free(buffer2);
    }

  //Clean up OpenSSL.
  EVP_cleanup();
  ERR_free_strings();

  return 0;
 } 
static int allocate_buffer_block(char **buffer, int length, int block_size)
 {
  int i=0;
  int j=0;
   
  /*
     Pad strings with extra block_size. To prevent buffer overflow, encrypting with cbc, the output
  buffer needs to be one block bigger than the input buffer.
  */
  if(length<block_size) 
    {
      length=2*block_size;
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
      i=length/block_size;
      j=length%block_size;
      if(j>0)
        {
          length=i*block_size+2*block_size;
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
          length=i*block_size+block_size;
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
static void handleErrors(void)
 {
  ERR_print_errors_fp(stderr);
  abort();
 }
static int encrypt_string(const char *string, int string_len, char *key, char *iv, char *buffer1)
 {
  EVP_CIPHER_CTX *ctx;
  int len;
  int ciphertext_len;

  if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

  if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (unsigned char *)key, (unsigned char *)iv))
    handleErrors();

  if(1 != EVP_EncryptUpdate(ctx, (unsigned char *)buffer1, &len, (unsigned char *)string, string_len))
    handleErrors();
  ciphertext_len = len;

  if(1 != EVP_EncryptFinal_ex(ctx, (unsigned char *)buffer1 + len, &len)) handleErrors();
  ciphertext_len += len;

  EVP_CIPHER_CTX_free(ctx);

  return ciphertext_len;
 }
static int decrypt_string(char *buffer1, int encrypt_len, char *key, char *iv, char *buffer2)
 {
  EVP_CIPHER_CTX *ctx;
  int len;
  int plaintext_len;

  if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

  if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (unsigned char *)key, (unsigned char *)iv))
    handleErrors();

  if(1 != EVP_DecryptUpdate(ctx, (unsigned char *)buffer2, &len, (unsigned char *)buffer1, encrypt_len))
    handleErrors();
  plaintext_len = len;

  if(1 != EVP_DecryptFinal_ex(ctx, (unsigned char *)buffer2 + len, &len)) handleErrors();
  plaintext_len += len;

  EVP_CIPHER_CTX_free(ctx);

  return plaintext_len;
 }
