#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h> /*this is the file structure, file open read close */
#include<linux/cdev.h> /* this is for character device, makes cdev avilable*/
#include<linux/semaphore.h> /* this is for the semaphore*/
#include<linux/uaccess.h> /*this is for copy_user vice vers*/

MODULE_LICENSE("GPL");    // This is specify that the code is open source

int chardev_init(void);   // initialising the device driver
void chardev_exit(void);  // releasing the device driver
static int device_open(struct inode *, struct file *);    // function for opening the device driver
static int device_close(struct inode *, struct file *);   // function for closing the device driver
static ssize_t device_read(struct file *, char *, size_t, loff_t *);            // return signed integer value as this is ssize_t so it will take care of 32 / 64 bit system we don't need to take care of that
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

/*new code*/
#define BUFFER_SIZE 1024
static char device_buffer[BUFFER_SIZE];         // to store the string that we enter
struct semaphore sem;                           // for locking and unlocking the device when in used
struct cdev *mcdev;                             /*this is the name of my char driver that i will be registering*/
int major_number;                               /* will store the major number extracted by dev_t*/
int ret;                                        /*used to return values*/
dev_t dev_num;                                    /*will hold the major number that the kernel gives*/

#define DEVICENAME "ourdevice"

/*inode reffers to the actual file on disk*/
static int device_open(struct inode *inode, struct file *filp) {
    if(down_interruptible(&sem) != 0) {
        printk(KERN_ALERT "ourdevice : the device has been opened by some other device, unable to open lock\n");
        return -1;
    }
    //buff_rptr = buff_wptr = device_buffer;
    printk(KERN_INFO "ourdevice : device opened succesfully\n");
    return 0;
}

static ssize_t device_read(struct file *fp, char *buff, size_t length, loff_t *ppos) {
    int maxbytes; /*maximum bytes that can be read from ppos to BUFFER_SIZE*/
    int bytes_to_read; /* gives the number of bytes to read*/
    int bytes_read;/*number of bytes actually read*/
    maxbytes = BUFFER_SIZE - *ppos;
    if(maxbytes > length)
        bytes_to_read = length;
    else
        bytes_to_read = maxbytes;
    if(bytes_to_read == 0)
        printk(KERN_INFO "ourdevice : Reached the end of the device\n");
    bytes_read = bytes_to_read - copy_to_user(buff, device_buffer + *ppos, bytes_to_read);
    printk(KERN_INFO "ourdevice : device has been read %d\n",bytes_read);
    *ppos += bytes_read;
    printk(KERN_INFO "ourdevice : device has been read\n");
    return bytes_read;
}

static ssize_t device_write(struct file *fp, const char *buff, size_t length, loff_t *ppos) {
    int maxbytes; /*maximum bytes that can be read from ppos to BUFFER_SIZE*/
    int bytes_to_write; /* gives the number of bytes to write*/
    int bytes_writen;/*number of bytes actually writen*/
    maxbytes = BUFFER_SIZE - *ppos;
    if(maxbytes > length)
        bytes_to_write = length;
    else
        bytes_to_write = maxbytes;
    bytes_writen = bytes_to_write - copy_from_user(device_buffer + *ppos, buff, bytes_to_write);
    printk(KERN_INFO "ourdevice : device has been written %d\n",bytes_writen);
    *ppos += bytes_writen;
    printk(KERN_INFO "ourdevice : device has been written\n");
    return bytes_writen;
}

static int device_close(struct inode *inode, struct file *filp) {
    up(&sem);
    printk(KERN_INFO "ourdevice : device has been closed\n");
    return ret;
}

struct file_operations fops = { /* these are the file operations provided by our driver */
    .owner = THIS_MODULE, /*prevents unloading when operations are in use*/
    .open = device_open,  /*to open the device*/
    .write = device_write, /*to write to the device*/
    .read = device_read, /*to read the device*/
    .release = device_close, /*to close the device*/
};


int chardev_init(void)
{
    printk(KERN_ALERT " Working\n");
    /* we will get the major number dynamically this is recommended please read ldd3*/
    ret = alloc_chrdev_region(&dev_num,0,1,DEVICENAME);
    if(ret < 0) {
        printk(KERN_ALERT " ourdevice : failed to allocate major number\n");
        return ret;
    }
    else
        printk(KERN_INFO " ourdevice : mjor number allocated succesful\n");
    major_number = MAJOR(dev_num);
    printk(KERN_INFO "ourdevice : major number of our device is %d\n",major_number);
    printk(KERN_INFO "ourdevice : to use mknod /dev/%s c %d 0\n",DEVICENAME,major_number);

    mcdev = cdev_alloc(); /*create, allocate and initialize our cdev structure*/
    mcdev->ops = &fops;   /*fops stand for our file operations*/
    mcdev->owner = THIS_MODULE;

    /*we have created and initialized our cdev structure now we need to add it to the kernel*/
    ret = cdev_add(mcdev,dev_num,1);
    if(ret < 0) {
        printk(KERN_ALERT "ourdevice : device adding to the kerknel failed\n");
        return ret;
    }
    else
        printk(KERN_INFO "ourdevice : device additin to the kernel succesful\n");
    sema_init(&sem,1);  /* initial value to one*/

    return 0;
}

void chardev_exit(void)
{
    cdev_del(mcdev); /*removing the structure that we added previously*/
    printk(KERN_INFO " ourdevice : removed the mcdev from kernel\n");

    unregister_chrdev_region(dev_num,1);
    printk(KERN_INFO "ourdevice : unregistered the device numbers\n");
    printk(KERN_ALERT " ourdevice : character driver is exiting\n");
}

MODULE_AUTHOR("SWAROOP");
MODULE_DESCRIPTION("A BASIC CHAR DRIVER");

module_init(chardev_init);
module_exit(chardev_exit);
