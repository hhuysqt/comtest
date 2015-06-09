#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h> /* File control definitions */
#include <errno.h>
#include <termios.h> /* POSIX terminal control definitions */

/*
 *  * 'open_port()' - Open serial port 1
 *   * Returns the file descriptor on success or -1 on error.
 *    */

int open_port(void)
{ int fd; /* File descriptor for the port */ 
	fd = open("/dev/ttyUSB1", O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		/*
		 *		 * Could not open the port.
		 *				 */
		perror("open_port: Unable to open /dev/ttyS0 -");
	}
	else
	{
		printf("Successfully opened it!\n");
		return (fd);
	}
}

int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio,oldtio;
    if  ( tcgetattr( fd,&oldtio)  !=  0) 
    { 
        perror("SetupSerial 1");
        return -1;
    }
	bzero( &newtio, sizeof( newtio ) );
    newtio.c_cflag  |=  CLOCAL | CREAD; 
	newtio.c_cflag &= ~CSIZE; 

    switch( nBits )
	{
	    case 7:
	        newtio.c_cflag |= CS7;
	        break;
	    case 8:
	        newtio.c_cflag |= CS8;
	        break;
    }

    switch( nEvent )
    {
	    case 'O':                     //奇校验
	        newtio.c_cflag |= PARENB;
	        newtio.c_cflag |= PARODD;
	        newtio.c_iflag |= (INPCK | ISTRIP);
	        break;
	    case 'E':                     //偶校验
	        newtio.c_iflag |= (INPCK | ISTRIP);
	        newtio.c_cflag |= PARENB;
	        newtio.c_cflag &= ~PARODD;
	        break;
	    case 'N':                    //无校验
	        newtio.c_cflag &= ~PARENB;
	        break;
    }

	switch( nSpeed )
    {
	    case 2400:
	        cfsetispeed(&newtio, B2400);
	        cfsetospeed(&newtio, B2400);
	        break;
	    case 4800:
	        cfsetispeed(&newtio, B4800);
	        cfsetospeed(&newtio, B4800);
	        break;
	    case 9600:
	        cfsetispeed(&newtio, B9600);
	        cfsetospeed(&newtio, B9600);
	        break;
	    case 115200:
	        cfsetispeed(&newtio, B115200);
	        cfsetospeed(&newtio, B115200);
	        break;
	    default:
	        cfsetispeed(&newtio, B9600);
	        cfsetospeed(&newtio, B9600);
	        break;
    }

	if( nStop == 1)
	{
		newtio.c_cflag &=  ~CSTOPB;
	}
	else if ( nStop == 2 )
	{
	     newtio.c_cflag |=  CSTOPB;
	}
	newtio.c_cc[VTIME]  = 0;
	newtio.c_cc[VMIN] = 0;
	tcflush(fd,TCIFLUSH);
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
	    perror("com set error");
	    return -1;
	}
	printf("set done!\n");
	return 0;
}

void main()
{
	int comID;
	char buff[1024];
	char comchar;
	int num;

	FILE *comF;
	comF = fopen("/dev/ttyUSB1", "r+");
	comID = open_port();
	if(comF != NULL)
	{
		printf("fopen succeed!\n");
		getchar();
    
		if(set_opt(comID, 115200, 8, 'N', 1) < 0)
		{
			printf("ERROR...\n");
			return;
		}
		printf("init succeeded.\n");
    
		fwrite("AT\r\n", 1, 4, comF);
		printf("Reading...\n");
    
		while(1)
		{
			num = fread(buff, sizeof(char), 1024, comF);
			buff[num] = '\0';
			printf("%d %s\n", num, buff);
			getchar();
		}
		fclose(comF);
	}
	else
	{
		printf("fopen failed..\n");
    
		if(set_opt(comID, 115200, 8, 'N', 1) < 0)
		{
			printf("ERROR...\n");
			return;
		}
    
		write(comID, "AT\r\n", 4);
		printf("Reading...\n");
    
		while(1)
		{
			fcntl(comID, F_SETFL, FNDELAY);
			num = read(comID, buff, sizeof(buff));
			fcntl(comID, F_SETFL, 0);
			printf("%d %s\r\n", num, buff);
		}
    
		close(comID);
	}
}
