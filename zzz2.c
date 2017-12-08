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
    int dataSize; 

    char serverIP[1024];
    int  serverPort; 

    if(!((argc == 3)||(argc == 5))){
      printf(" reqire 2 or 4 arguments: cmd filename [dest_ip port_num]\n"); 
      exit(1);  
    }

    printf(" argv[1] = %s\n", argv[1]); 
    printf(" argv[2] = %s\n", argv[2]); 

    if(argc == 5){
      printf(" argv[3] = %s\n", argv[3]); 
      printf(" argv[4] = %s\n", argv[4]); 

      strcpy(serverIP, argv[3]); 
      serverPort = atoi(argv[4]); 
    }
    else{
      strcpy(serverIP, "192.168.6.12"); 
      serverPort = 5000; 
    }

    strcpy(str1, COMPAREDATA); 

    if(strncmp(argv[1], str1, OPTIONSIZE)==0){

      printf(" load datafile %s\n", argv[2]); 
      
      gettimeofday(&begin, NULL);

      int fd; 

      if((fd = open(argv[2], O_RDONLY)) < 0){
        printf("cannot open %s\n", argv[2]); 
        exit(1); 
      } 

      printf(" reading contents of %s to memory\n", argv[2]); 

      x = 0; 
      while(1){
        n = read(fd, &a[x], DATABLOCK);  
        if (n == 0){
          printf("end of file is reached\n"); 
          break; 
        }
        else if(n < 0){
          printf("error while reading abort!\n"); 
          exit(1); 
        }
        x += n; 
      }

      dataSize = x; 

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf(" loading input file time = %lf loaded file size = %d\n", elapsed, dataSize); 

      close(fd); 

      gettimeofday(&begin, NULL);


      uLong ucompSize = dataSize; 
      uLong compSize = compressBound(ucompSize);

      printf(" before compress: uncompress size = %ld  initial compress size = %ld\n", ucompSize, compSize); 
      // Deflate
      compress((Bytef *)b, &compSize, (Bytef *)a, ucompSize);

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf(" after compress: compress time = %lf compress Size = %ld, compression ratio = %lf\n", 
           elapsed, compSize, ((double)((double) ucompSize/(double) compSize))); 

      gettimeofday(&begin, NULL);
      // Inflate
      uncompress((Bytef *)c, &ucompSize, (Bytef *)b, compSize);

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf(" after uncompress: uncompress time = %lf uncompress size = %ld \n", elapsed, ucompSize); 

      // verify c and a
      for (x = 0; x < dataSize; x++){
        if (c[x] != a[x]){
          printf("compression and decompression error\n"); 
          break; 
        }
      } 
      if (x == dataSize) printf(" verification: compression correct\n"); 

    }

    strcpy(str1, CLIENTCMDDATA); 

    if(strncmp(argv[1], str1, OPTIONSIZE)==0){

      int sockfd = 0, n = 0;
      struct sockaddr_in serv_addr; 

      printf("1. Client load datafile to compress\n"); 
      
      gettimeofday(&begin, NULL);

      int fd; 

      if((fd = open(argv[2], O_RDONLY)) < 0){
        printf("cannot open %s\n", argv[2]); 
        exit(1); 
      } 

      printf(" reading contents of %s to memory\n", argv[2]); 

      x = 0; 
      while(1){
        n = read(fd, &a[x], DATABLOCK);  
        if (n == 0){
          printf("end of file is reached\n"); 
          break; 
        }
        else if(n < 0){
          printf("error while reading abort!\n"); 
          exit(1); 
        }
        x += n; 
      }

      dataSize = x; 

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf(" loading input file time = %lf loaded file size = %d\n", elapsed, dataSize); 

      close(fd); 

      printf("2. Client compress data file in memory\n"); 
      
      gettimeofday(&begin, NULL);

      uLong ucompSize = dataSize; 
      uLong compSize = compressBound(ucompSize);

      printf(" before compress: uncompress size = %ld  initial compress size = %ld\n", ucompSize, compSize); 
      // Deflate
      compress((Bytef *)b, &compSize, (Bytef *)a, ucompSize);

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf(" after compress: compress time = %lf compress Size = %ld, compression ratio = %lf\n", 
           elapsed, compSize, ((double)((double) ucompSize/(double) compSize))); 

      if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
         printf("\n Error : Could not create socket \n");
         return 1;
      } 

      memset(&serv_addr, '0', sizeof(serv_addr)); 

      serv_addr.sin_family = AF_INET;
      serv_addr.sin_port = htons(serverPort); 

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

      printf(" data transfer time of compressed data:%lf\n", elapsed); 

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

      time_t ticks; 

      printf("1. server load datafile for verification\n"); 
      
      gettimeofday(&begin, NULL);

      int fd; 

      if((fd = open(argv[2], O_RDONLY)) < 0){
        printf("cannot open %s\n", argv[2]); 
        exit(1); 
      } 

      printf(" reading contents of %s\n", argv[2]); 

      x = 0; 
      while(1){
        n = read(fd, &a[x], DATABLOCK);  
        if (n == 0){
          printf("end of file is reached\n"); 
          break; 
        }
        else if(n < 0){
          printf("error while reading abort!\n"); 
          exit(1); 
        }
        x += n; 
      }

      dataSize = x; 

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf(" loading input file time = %lf loaded file size = %d\n", elapsed, dataSize); 

      close(fd); 

      printf("2. server wait for connection from client\n"); 

      listenfd = socket(AF_INET, SOCK_STREAM, 0);
      memset(&serv_addr, '0', sizeof(serv_addr));

      serv_addr.sin_family = AF_INET;
      serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
      serv_addr.sin_port = htons(serverPort); 

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

            printf(" time to receive transmitted compressed data = %lf\n", elapsed); 

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

      printf(" server after uncompress: uncompress time = %lf uncompress size = %ld \n", elapsed, ucompSize); 

      printf("4. verify compressed data\n"); 

      // verify c and a
      for (x = 0; x < dataSize; x++){
        if (c[x] != a[x]){
          printf("compression and decompression error\n"); 
          break; 
        }
      } 
      if (x == dataSize) printf(" verification: compression correct\n"); 
    }

    strcpy(str1, CLIENTNOCMPR); 

    if(strncmp(argv[1], str1,OPTIONSIZE)==0){

      int sockfd = 0, n = 0;
      struct sockaddr_in serv_addr; 

      printf("1. Client load datafile to compress\n"); 
      
      gettimeofday(&begin, NULL);

      int fd; 

      if((fd = open(argv[2], O_RDONLY)) < 0){
        printf("cannot open %s\n", argv[2]); 
        exit(1); 
      } 

      printf("reading contents of %s\n", argv[2]); 

      x = 0; 
      while(1){
        n = read(fd, &a[x], DATABLOCK);  
        if (n == 0){
          printf("end of file is reached\n"); 
          break; 
        }
        else if(n < 0){
          printf("error while reading abort!\n"); 
          exit(1); 
        }
        x += n; 
      }

      dataSize = x; 

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf("loading input file time = %lf loaded file size = %d\n", elapsed, dataSize); 

      close(fd); 

      if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
         printf("\n Error : Could not create socket \n");
         return 1;
      } 

      memset(&serv_addr, '0', sizeof(serv_addr)); 

      serv_addr.sin_family = AF_INET;
      serv_addr.sin_port = htons(serverPort); 

      if(inet_pton(AF_INET, serverIP, &serv_addr.sin_addr)<=0){
         printf("\n inet_pton error occured\n");
         return 1;
      } 

      if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
         printf("\n Error : Connect Failed \n");
         return 1;
      } 

      uLong compSize = dataSize; 
      uLong dataLeft = dataSize; 
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

      close(sockfd); 

      printf(" amount of 4096-sized row sent = %d\n", irow); 

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf(" transfer time of uncompressed data = %lf\n", elapsed); 

      //read_full(sockfd, &dataReceived, sizeof(uLong)); 

      //printf(" amount of data server received = %ld\n", dataReceived); 

    }

    strcpy(str1, SERVERNOCMPR); 

    if(strncmp(argv[1], str1,OPTIONSIZE)==0){

      uLong compSize = 0; 
      uLong ucompSize = 0; 

      int listenfd = 0, connfd = 0;
      struct sockaddr_in serv_addr; 

      time_t ticks; 

      printf("1. server load datafile for verification\n"); 
      
      gettimeofday(&begin, NULL);

      int fd; 

      if((fd = open(argv[2], O_RDONLY)) < 0){
        printf("cannot open %s\n", argv[2]); 
        exit(1); 
      } 

      printf("reading contents of %s\n", argv[2]); 

      x = 0; 
      while(1){
        n = read(fd, &a[x], DATABLOCK);  
        if (n == 0){
          printf("end of file is reached\n"); 
          break; 
        }
        else if(n < 0){
          printf("error while reading abort!\n"); 
          exit(1); 
        }
        x += n; 
      }

      dataSize = x; 

      gettimeofday(&end, NULL);

      elapsed = (end.tv_sec - begin.tv_sec) + 
              (((double)(end.tv_usec - begin.tv_usec))/1000000.0);

      printf("loading input file time = %lf loaded file size = %d\n", elapsed, dataSize); 

      close(fd); 

      printf("2. server wait for connection from client\n"); 

      listenfd = socket(AF_INET, SOCK_STREAM, 0);
      memset(&serv_addr, '0', sizeof(serv_addr));

      serv_addr.sin_family = AF_INET;
      serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
      serv_addr.sin_port = htons(serverPort); 

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

            printf(" time receiving transmitted uncompressed data = %lf\n", elapsed); 

            printf(" amount of data received = %ld (%d rows of 4096)\n", dataReceived, irow); 

            //write_full(connfd, &dataReceived, sizeof(uLong)); 

            close(connfd);
            sleep(1);

            break; 
      }

      printf("3. verify compressed data\n"); 

      // verify c and a
      for (x = 0; x < dataSize; x++){
        if (b[x] != a[x]){
          printf("data corrpt due to transmission error\n"); 
          break; 
        }
      } 
      if (x == dataSize) printf("transmission correct\n"); 
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
        else if (ret == 0){
            return ret; 
        }

        count -= ret;
        buf += ret;
        total += ret;
    }

    return total;
}

