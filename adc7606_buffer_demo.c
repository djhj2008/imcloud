#include <linux/input.h>  
#include <fcntl.h>  
#include <stdio.h>  
#include <stdint.h>  
#include <stdlib.h>  
#include <linux/rtc.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <getopt.h>

#define  ADC_DEV_NAME    "adc7606_regs"  
#define  ADC_DEV_PATH_NAME    "/dev/"  

#define ADC_SAMPLE_SIZE 192
//#define ADC_SAMPLE_SIZE 3

#define DETAILED_VERSION 1

struct ping_buffer_data{
	//struct rtc_time tm;
	unsigned long long time_stamp;
	unsigned int pnumber[2];
	unsigned int sample[ADC_SAMPLE_SIZE];
};


int openInputDev(const char* inputName)
{
    int fd = -1;
    char devname[128];
    int i;

	for(i = 0; i< 5; i++){
		sprintf(devname, "%s%s", ADC_DEV_PATH_NAME, inputName);

		fd = open(devname, O_RDONLY);
		if(fd >= 0){
			break;
		}
	}
	if(i >= 5)
		return -1;
	close(fd);

    return 0;
}

int sysInputScan(void)  
{  
    int l_ret = -1;  
    int i = 0;  
#if DETAILED_VERSION
    int ch = 0;
    int sample = 0;
    //int sample_num = 0;
#endif      
    int dev_fd  = 0;  
    
    struct ping_buffer_data ping_data;  
    char devName[128];

		sprintf(devName, "%s%s", ADC_DEV_PATH_NAME, ADC_DEV_NAME);

    dev_fd = open(devName, O_RDONLY);  
    if(dev_fd <= 0)
    {  
        printf("adc7606 open devName : %s error\n", devName);  
        return l_ret;  
    }  

    if (ioctl(dev_fd, 1, 1)) {
        printf("adc7606 ioctl set enable failed!\n");  
        return -1;
    }

    while(1)  
    {  
        l_ret = lseek(dev_fd, 0, SEEK_SET);  
        l_ret = read(dev_fd, &ping_data, sizeof(ping_data));  
          
        if(l_ret)  
        {
			   //clock_t start,finish;
			   //double totaltime = 0;
			   //start = clock();

#if DETAILED_VERSION
				#if 0
   			printf("%d-%d-%d %d:%d:%d \n",
				ping_data.tm.tm_year,
				ping_data.tm.tm_mon,
				ping_data.tm.tm_mday,
				ping_data.tm.tm_hour,
				ping_data.tm.tm_min,
				ping_data.tm.tm_sec);
				#else
   			printf("time stamp: %lld \n", ping_data.time_stamp);
				#endif
#endif
        for(i = 0; i< 2; i++)
					printf("pnumber[%d]= 0x%x\n", i, ping_data.pnumber[i]);  

#if DETAILED_VERSION
				for(ch = 0; ch < 6; ch++){
					int flag = ch % 2;

					for(sample = 0; sample < 64; sample++){
							unsigned short old_val;
							short val;
							if(flag == 0){
								old_val = ping_data.sample[(sample * 3) + (ch/2)] & 0xffff;
								//printf("ch : %d (sample * 3) + (ch/2) = %d\n", ch, (sample * 3) + (ch/2));
							}
							else{
								old_val = (ping_data.sample[(sample * 3) + (ch/2)] >> 16) & 0xffff;
								//printf("ch : %d (sample * 3) + (ch/2) = %d\n", ch, (sample * 3) + (ch/2));
							}

							val = old_val;
		         	printf("CH[%d] = %d\n", ch, val);

						#if 0
						if(flag == 0){
		         	printf("CH[%d] = 0x%x\n", ch, ping_data.sample[sample_num + sample * 3] & 0xffff);
		        }
		        else{
		         	printf("CH[%d] = 0x%x\n", ch, (ping_data.sample[sample_num - 1 + sample * 3] >> 16) & 0xffff);
		        }
		        #endif
		      }
		    }
#endif
				//finish = clock();
				//totaltime = (double)(finish-start)/CLOCKS_PER_SEC;
		    //printf("totaltime = %f\n", totaltime);

	      #if 0
				for(sample = 0; sample < 64; sample++){
						//sample = 0;
		        for(i = 0, ch = 0; i< 3; i++, ch++){
		         	printf("user sample[%d] CH%d = 0x%x\n", sample, ch * 2, ping_data.sample[i + sample * 3] & 0xffff);
		         	printf("user sample[%d] CH%d = 0x%x\n", sample, ch * 2 + 1, (ping_data.sample[i + sample * 3] >> 16) & 0xffff);
						}
	      }

				for(sample = 63 - 3; sample < 64; sample++){
						//sample = 63;
		        //for(i = ADC_SAMPLE_SIZE - 3, ch = 0; i< ADC_SAMPLE_SIZE; i++, ch++){
		        for(i = 0, ch = 0; i< 3; i++, ch++){
		         	printf("user sample[%d] CH%d = 0x%x\n", sample, ch * 2, ping_data.sample[i + sample * 3] & 0xffff);
		         	printf("user sample[%d] CH%d = 0x%x\n", sample, ch * 2 + 1, (ping_data.sample[i + sample * 3] >> 16) & 0xffff);
		        }
    		}
    		#endif
    	  }
  
    }  

    if (ioctl(dev_fd, 1, 0)) {
        printf("adc7606 ioctl set disable failed!\n");  
        return -1;
    }
    close(dev_fd);  
      
    return l_ret;  
      
}  
  
int openEnable(int enable)
{
    FILE* fd;
    char devname[128];
    char reg_value[32];

		sprintf(devname, "%s", "/sys/bus/platform/devices/ff228000.register/register_regs");
		/*base addr: 0x0, bit1, enable: 0 | 1 */
    sprintf(reg_value,"0 1 %d", enable);

		fd = fopen(devname, "w+");
		if(fd != NULL){
        fwrite(reg_value, 1, strlen(reg_value) + 1, fd);
		}else{
			printf("adc7606 enable error : %s\n", devname);  
		}

		fclose(fd);

    return 0;
}

int setRange(int val)
{
    FILE* fd;
    char devname[128];
    char reg_value[32];

		if(val != 0 && val != 1){
			printf("adc7606 setRange val invalid : %d\n", val);
			return 0;
		}

		sprintf(devname, "%s", "/sys/bus/platform/devices/ff228000.register/register_regs");
		/*base addr: 0x0, bit2, ADC input range (0: +/-5V, 1: +/-10V) */
    sprintf(reg_value,"0 2 %d", val);

		fd = fopen(devname, "w+");
		if(fd != NULL){
        fwrite(reg_value, 1, strlen(reg_value) + 1, fd);
		}else{
			printf("adc7606 setRange error : %s\n", devname);  
		}

		fclose(fd);

    return 0;
}

static void print_usage(const char *prog)
{
	printf("Usage: %s [-red]\n", prog);
	puts("  -e --enable   DC calibration enable\n"
	     "  -d --disable  DC calibration disable\n"
	     "  -r --range    ADC input range (0: +/-5V, 1: +/-10V)\n");
	exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "range",   1, 0, 'r' },
			{ "enable",  0, 0, 'e' },
			{ "disable", 0, 0, 'd' },
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "r:ed", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'e':
			openEnable(1);
			break;
		case 'd':
			openEnable(0);
			break;
		case 'r':
			setRange(atoi(optarg));
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
}

int main(int arg, char *arc[])  
{
	int ret = 0;
	//int enable = 0;
	
	//printf("adc7606 dev test arg = %d\n",arg);  
	ret = openInputDev(ADC_DEV_NAME);
	if(ret){
		printf("adc7606 open device error\n");  
		return 0;
	} else {
		printf("adc7606 input device dir: %s%s\n", ADC_DEV_PATH_NAME, ADC_DEV_NAME);  
	}
	#if 0
	if(arg == 2){
		switch ( atoi(arc[1])) {
		case 0:
			enable = 0;
			break;
		case 1:
			enable = 1;
			break;
		default:
			print_usage();
			break;
		}
		openEnable(enable);
	}
	#else
	parse_opts(arg, arc);
	#endif
	while(1)
	{  
		ret = sysInputScan(); 
		if(ret <=0)
			return 0;
	}  
	return 0;  
}  

