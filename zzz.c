#include <stdio.h>
#include <zlib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h> 

#define DATASIZE	256*1024*1024
#define DATABLOCK       1024
#define DATAWRITEFOLDS  256*1024

char a[DATASIZE];
char b[DATASIZE];
char c[DATASIZE];

#define OPTIONSIZE  4
#define CREATEDATA  "data"
#define COMPAREDATA  "cmpd"
#define CLIENTCMDDATA  "clid"
#define SERVERCMDDATA  "serd"
#define CLIENTNOCMPR  "clin"
#define SERVERNOCMPR  "sern"

char str1[10]; 

struct timeval begin, end;
double elapsed; 

int write_full(int fd, const void *buf, size_t count);
int read_full(int fd, void *buf, size_t count);

int main(int argc, char *argv[]){

    int x, n; 
    int tx_size; 

    if(argc != 2){
      printf("reqire 2 arguments\n"); 
      exit(1);  
    }

    printf(" argv[1] = %s\n", argv[1]); 

    strcpy(str1, CREATEDATA); 

    if(strncmp(argv[1], str1,OPTIONSIZE)==0){

      // initialize a 
      for (x = 0; x < DATASIZE; x++){
        a[x] = (rand() % 127); 
        // if ( x < 10 ) printf(" a [ %d ] = %d\n", x, a[x]); 
      }

      int fd = open("datafile", O_CREAT | O_WRONLY | O_APPEND, 0755); 

      if (fd == -1){
        printf("\n open() failed with error [%s]\n",strerror(errno));
        exit(1);
      }
      else{
        printf("\n Open() Successful\n");
      }

      for (x = 0; x < DATASIZE; x += DATABLOCK){
        n = write(fd, &a[x], DATABLOCK); 
      }

      close(fd); 

      printf("fd = %d arraysize = %d  writexsize = %d n = %d\n", fd, DATASIZE, x, n ); 

      exit(0); 
    }

    strcpy(str1, COMPAREDATA); 

    if(strncmp(argv[1], str1,OPTIONSIZE)==0){

      printf("load datafile\n"); 
      
      gettimeofday(&begin, NULL);

      int fd = open("./datafile", O_RDONLY); 

      for (x = 0; x < DATASIZE; x += DATABLOCK){
        read(fd, &a[x], DATABLOCK); 
      }

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf("readfileTime:%lf: loaded arraysize = %d  readxsize = %d\n", elapsed, DATASIZE, x ); 

      close(fd); 

      gettimeofday(&begin, NULL);

      uLong ucompSize = DATASIZE; 
      uLong compSize = compressBound(ucompSize);

      printf(" before compress: ucompSize = %ld , compSize = %ld\n", ucompSize, compSize); 
      // Deflate
      compress((Bytef *)b, &compSize, (Bytef *)a, ucompSize);

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf(" after compress:compressTime:%lf: compSize = %ld\n", elapsed, compSize); 

      gettimeofday(&begin, NULL);
      // Inflate
      uncompress((Bytef *)c, &ucompSize, (Bytef *)b, compSize);

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf(" after uncompress:uncompressTime:%lf: ucompSize = %ld \n", elapsed, ucompSize); 

      // verify c and a
      for (x = 0; x < DATASIZE; x++){
        if (c[x] != a[x]){
          printf("compression and decompression error\n"); 
          break; 
        }
      } 
      if (x == DATASIZE) printf("compression correct\n"); 

    }

    strcpy(str1, CLIENTCMDDATA); 

    if(strncmp(argv[1], str1,OPTIONSIZE)==0){

      int sockfd = 0, n = 0;
      char recvBuff[1024];
      char serverIP[1024];
      struct sockaddr_in serv_addr; 

      printf("1. Client load datafile to compress\n"); 
      
      gettimeofday(&begin, NULL);

      int fd = open("./datafile", O_RDONLY); 

      for (x = 0; x < DATASIZE; x += DATABLOCK){
        read(fd, &a[x], DATABLOCK); 
      }

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf("readfileTime:%lf: loaded arraysize = %d  readxsize = %d\n", elapsed, DATASIZE, x ); 

      close(fd); 

      printf("2. Client compress data file in memory\n"); 
      
      gettimeofday(&begin, NULL);

      uLong ucompSize = DATASIZE; 
      uLong compSize = compressBound(ucompSize);

      printf(" before compress: ucompSize = %ld , compSize = %ld\n", ucompSize, compSize); 
      // Deflate
      compress((Bytef *)b, &compSize, (Bytef *)a, ucompSize);

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf(" after compress:compressTime:%lf: compSize = %ld\n", elapsed, compSize); 

      strcpy(serverIP, "192.168.6.13"); 

      memset(recvBuff, '0',sizeof(recvBuff));
      if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
         printf("\n Error : Could not create socket \n");
         return 1;
      } 

      memset(&serv_addr, '0', sizeof(serv_addr)); 

      serv_addr.sin_family = AF_INET;
      serv_addr.sin_port = htons(5000); 

      if(inet_pton(AF_INET, serverIP, &serv_addr.sin_addr)<=0){
         printf("\n inet_pton error occured\n");
         return 1;
      } 

      if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
         printf("\n Error : Connect Failed \n");
         return 1;
      } 

      uLong dataLeft = compSize; 
      uLong dataReceived = 0; 

      int inx = 0; 
      int irow = 0; 

      write_full(sockfd, &compSize, sizeof(compSize)); 

#define TX_BLOCK_SIZE 4096

      gettimeofday(&begin, NULL);

      while (dataLeft > 0){
        if (dataLeft >= TX_BLOCK_SIZE){
           tx_size = TX_BLOCK_SIZE; 
           dataLeft -= TX_BLOCK_SIZE; 
        }
        else{
           tx_size = dataLeft; 
           dataLeft = 0; 
        }

        write_full(sockfd, &b[inx], tx_size); 

        inx += tx_size; 
        irow++; 
      }
      printf(" amount of 4096-sized row sent = %d\n", irow); 

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf("TxTime of Compressed data:%lf\n", elapsed); 

      read_full(sockfd, &dataReceived, sizeof(uLong)); 

      printf(" amount of data server received = %ld\n", dataReceived); 

      close(sockfd); 
    }

    strcpy(str1, SERVERCMDDATA); 

    if(strncmp(argv[1], str1,OPTIONSIZE)==0){

      uLong ucompSize = DATASIZE; 
      uLong compSize = compressBound(ucompSize);

      int listenfd = 0, connfd = 0;
      struct sockaddr_in serv_addr; 

      char sendBuff[1025];
      time_t ticks; 

      printf("1. server load datafile for verification\n"); 
      
      gettimeofday(&begin, NULL);

      int fd = open("./datafile", O_RDONLY); 

      for (x = 0; x < DATASIZE; x += DATABLOCK){
        read(fd, &a[x], DATABLOCK); 
      }

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf("readfileTime:%lf: loaded arraysize = %d  readxsize = %d\n", elapsed, DATASIZE, x ); 

      close(fd); 

      printf("2. server wait for connection from client\n"); 

      listenfd = socket(AF_INET, SOCK_STREAM, 0);
      memset(&serv_addr, '0', sizeof(serv_addr));
      memset(sendBuff, '0', sizeof(sendBuff)); 

      serv_addr.sin_family = AF_INET;
      serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
      serv_addr.sin_port = htons(5000); 

      bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

      listen(listenfd, 10); 

      while(1) {
            connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 

            int inx = 0; 
            int irow = 0; 

            read_full(connfd, &compSize, sizeof(compSize)); 
            printf(" received compSize = %ld\n", compSize); 

            uLong dataLeft = compSize; 
            uLong dataReceived = 0; 

#define TX_BLOCK_SIZE 4096
            gettimeofday(&begin, NULL);

            while (dataLeft > 0){
              if (dataLeft >= TX_BLOCK_SIZE){
                tx_size = TX_BLOCK_SIZE; 
                dataLeft -= TX_BLOCK_SIZE; 
              }
              else{
                tx_size = dataLeft; 
                dataLeft = 0; 
              }

              dataReceived += read_full(connfd, &b[inx], tx_size); 

              inx += tx_size; 
              irow++; 
            }

            gettimeofday(&end, NULL);

            elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

            printf("Time to receive Transmitted Compressed data:%lf\n", elapsed); 

            printf(" amount of data received = %ld (%d rows of 4096)\n", dataReceived, irow); 

            write_full(connfd, &dataReceived, sizeof(uLong)); 

            close(connfd);
            sleep(1);

            break; 
      }

      printf("3. server uncompress received data\n"); 

      gettimeofday(&begin, NULL);
      // Inflate
      uncompress((Bytef *)c, &ucompSize, (Bytef *)b, compSize);

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf(" server after uncompress:uncompressTime:%lf: ucompSize = %ld \n", elapsed, ucompSize); 

      printf("4. verify compressed data\n"); 

      // verify c and a
      for (x = 0; x < DATASIZE; x++){
        if (c[x] != a[x]){
          printf("compression and decompression error\n"); 
          break; 
        }
      } 
      if (x == DATASIZE) printf("compression correct\n"); 
    }

    strcpy(str1, COMPAREDATA); 

    if(strncmp(argv[1], str1,OPTIONSIZE)==0){

      printf("load datafile\n"); 
      
      gettimeofday(&begin, NULL);

      int fd = open("./datafile", O_RDONLY); 

      for (x = 0; x < DATASIZE; x += DATABLOCK){
        read(fd, &a[x], DATABLOCK); 
      }

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf("readfileTime:%lf: loaded arraysize = %d  readxsize = %d\n", elapsed, DATASIZE, x ); 

      close(fd); 

      gettimeofday(&begin, NULL);

      uLong ucompSize = DATASIZE; 
      uLong compSize = compressBound(ucompSize);

      printf(" before compress: ucompSize = %ld , compSize = %ld\n", ucompSize, compSize); 
      // Deflate
      compress((Bytef *)b, &compSize, (Bytef *)a, ucompSize);

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf(" after compress:compressTime:%lf: compSize = %ld\n", elapsed, compSize); 

      gettimeofday(&begin, NULL);
      // Inflate
      uncompress((Bytef *)c, &ucompSize, (Bytef *)b, compSize);

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf(" after uncompress:uncompressTime:%lf: ucompSize = %ld \n", elapsed, ucompSize); 

      // verify c and a
      for (x = 0; x < DATASIZE; x++){
        if (c[x] != a[x]){
          printf("compression and decompression error\n"); 
          break; 
        }
      } 
      if (x == DATASIZE) printf("compression correct\n"); 

    }

    strcpy(str1, CLIENTNOCMPR); 

    if(strncmp(argv[1], str1,OPTIONSIZE)==0){

      int sockfd = 0, n = 0;
      char recvBuff[1024];
      char serverIP[1024];
      struct sockaddr_in serv_addr; 

      printf("1. Client load datafile to compress\n"); 
      
      gettimeofday(&begin, NULL);

      int fd = open("./datafile", O_RDONLY); 

      for (x = 0; x < DATASIZE; x += DATABLOCK){
        read(fd, &a[x], DATABLOCK); 
      }

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf("readfileTime:%lf: loaded arraysize = %d  readxsize = %d\n", elapsed, DATASIZE, x ); 

      close(fd); 


      strcpy(serverIP, "192.168.6.13"); 

      memset(recvBuff, '0',sizeof(recvBuff));
      if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
         printf("\n Error : Could not create socket \n");
         return 1;
      } 

      memset(&serv_addr, '0', sizeof(serv_addr)); 

      serv_addr.sin_family = AF_INET;
      serv_addr.sin_port = htons(5000); 

      if(inet_pton(AF_INET, serverIP, &serv_addr.sin_addr)<=0){
         printf("\n inet_pton error occured\n");
         return 1;
      } 

      if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
         printf("\n Error : Connect Failed \n");
         return 1;
      } 

      uLong compSize = DATASIZE; 
      uLong dataLeft = DATASIZE; 
      uLong dataReceived = 0; 

      int inx = 0; 
      int irow = 0; 

      write_full(sockfd, &compSize, sizeof(compSize)); 

#define TX_BLOCK_SIZE 4096

      gettimeofday(&begin, NULL);

      while (dataLeft > 0){
        if (dataLeft >= TX_BLOCK_SIZE){
           tx_size = TX_BLOCK_SIZE; 
           dataLeft -= TX_BLOCK_SIZE; 
        }
        else{
           tx_size = dataLeft; 
           dataLeft = 0; 
        }

        write_full(sockfd, &a[inx], tx_size); 

        inx += tx_size; 
        irow++; 
      }
      printf(" amount of 4096-sized row sent = %d\n", irow); 

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf("TxTime of uncompressed data:%lf\n", elapsed); 

      read_full(sockfd, &dataReceived, sizeof(uLong)); 

      printf(" amount of data server received = %ld\n", dataReceived); 

      close(sockfd); 
    }

    strcpy(str1, SERVERNOCMPR); 

    if(strncmp(argv[1], str1,OPTIONSIZE)==0){

      uLong compSize = DATASIZE; 
      uLong ucompSize = DATASIZE; 

      int listenfd = 0, connfd = 0;
      struct sockaddr_in serv_addr; 

      char sendBuff[1025];
      time_t ticks; 

      printf("1. server load datafile for verification\n"); 
      
      gettimeofday(&begin, NULL);

      int fd = open("./datafile", O_RDONLY); 

      for (x = 0; x < DATASIZE; x += DATABLOCK){
        read(fd, &a[x], DATABLOCK); 
      }

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf("readfileTime:%lf: loaded arraysize = %d  readxsize = %d\n", elapsed, DATASIZE, x ); 

      close(fd); 

      printf("2. server wait for connection from client\n"); 

      listenfd = socket(AF_INET, SOCK_STREAM, 0);
      memset(&serv_addr, '0', sizeof(serv_addr));
      memset(sendBuff, '0', sizeof(sendBuff)); 

      serv_addr.sin_family = AF_INET;
      serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
      serv_addr.sin_port = htons(5000); 

      bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

      listen(listenfd, 10); 

      while(1) {
            connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 

            int inx = 0; 
            int irow = 0; 

            read_full(connfd, &compSize, sizeof(compSize)); 
            printf(" received compSize = %ld\n", compSize); 

            uLong dataLeft = compSize; 
            uLong dataReceived = 0; 

#define TX_BLOCK_SIZE 4096
            gettimeofday(&begin, NULL);

            while (dataLeft > 0){
              if (dataLeft >= TX_BLOCK_SIZE){
                tx_size = TX_BLOCK_SIZE; 
                dataLeft -= TX_BLOCK_SIZE; 
              }
              else{
                tx_size = dataLeft; 
                dataLeft = 0; 
              }

              dataReceived += read_full(connfd, &b[inx], tx_size); 

              inx += tx_size; 
              irow++; 
            }

            gettimeofday(&end, NULL);

            elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

            printf("Time to receive Transmitted uncompressed data:%lf\n", elapsed); 

            printf(" amount of data received = %ld (%d rows of 4096)\n", dataReceived, irow); 

            write_full(connfd, &dataReceived, sizeof(uLong)); 

            close(connfd);
            sleep(1);

            break; 
      }

      printf("3. verify compressed data\n"); 

      // verify c and a
      for (x = 0; x < DATASIZE; x++){
        if (b[x] != a[x]){
          printf("data corrpt due to transmission error\n"); 
          break; 
        }
      } 
      if (x == DATASIZE) printf("transmission correct\n"); 
    }
}

int write_full(int fd, const void *buf, size_t count){
    ssize_t ret = 0;
    ssize_t total = 0;

    while (count) {
        ret = write(fd, buf, count);
        if (ret < 0) {
            if (errno == EINTR){
                continue;
            }
            printf("write error errno=%d fd=%d\n", errno, fd);
            return ret;
        }

        count -= ret;
        buf += ret;
        total += ret;
    }

    return total;
}

int read_full(int fd, void *buf, size_t count){
    ssize_t ret = 0;
    ssize_t total = 0;

    while (count) {
        ret = read(fd, buf, count);
        if (ret < 0) {
            if (errno == EINTR){
                continue;
            }
            printf("read error errno=%d fd=%d\n", errno, fd);
            return ret;
        }

        count -= ret;
        buf += ret;
        total += ret;
    }

    return total;
}

