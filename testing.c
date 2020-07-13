#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include<unistd.h>

#include <linux/kernel.h>
#include <linux/fs.h>
// #include <asm/uaccess.h>

#define DEVICE "/dev/chardevice"
//#define DEVICE "swaroop.txt"
int debug = 1, fd = 0;
loff_t pos = 0;

int write_device() {
  // printf("%d" , fd);
    int write_length = 0;
    ssize_t ret;
        char *data = (char *)malloc(1024 * sizeof(char));
        // char data[1024];
    printf("please enter the data to write into device\n");
    scanf( " %[^\n]",data); /* a space added after"so that it reads white space, %[^\n] is addeed so that it takes input until new line*/
    write_length = strlen(data);
    if(debug)printf("the length of dat written = %d\n",write_length);
    ret = write(fd, data, write_length);
    // ret = vfs_write(fd, data, strlen(data), &fd.f_pos);
    if(ret == -1)
        printf("writting failed\n");
    else
        printf("writting success\n");
    if(debug){ fflush(stdout);/*not to miss any log*/ }
    free(data);
return 0;
}

int read_device() {
    int read_length = 0;
    ssize_t ret;
        char *data = (char *)malloc(1024 * sizeof(char));
    printf("enter the length of the buffer to read\n");
    scanf("%d",&read_length);
    if(debug)printf("the read length selected is %d\n",read_length);
    memset(data,0,sizeof(data));
    data[0] = '\0';
    ret = read(fd,data,read_length);
    printf("DEVICE_READ : %s\n",data);
    if(ret == -1)
        printf("reading failed\n");
    else
        printf("reading success\n");
    if(debug)fflush(stdout);/*not to miss any log*/
    free(data);
return 0;
}



int main()
{
    int value = 0;
    if(access(DEVICE, F_OK) == -1) {
        printf("module %s not loaded\n",DEVICE);
        return 0;
    }
    else
        printf("module %s loaded, will be used\n",DEVICE);

    while(1) {
    printf("\nEnter the choice : \n 1 . Write \n 2. Read \n");
        scanf("%d",&value);
        switch(value) {
            case 1 :
            {
            printf("write option selected\n");
            // fd = open(DEVICE, O_RDWR);
            fd = open(DEVICE, O_CREAT | O_WRONLY | O_APPEND, S_IRWXU );
            write_device();
            close(fd); /*closing the device*/
            break;
          }
            case 2 :
            {printf("read option selected\n");
            /* dont know why but i am suppoesed to open it for writing and close it, i cant keep  open and read.
            its not working, need to sort out why is that so */
            fd = open(DEVICE, O_RDWR);
            // printf("%d\n" , fd );
            read_device();
            close(fd); /*closing the device*/
            break;}

            default :
            { printf("unknown  option selected, please enter right one\n");
            break;}
        }
    }
    return 0;
}
