#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>
#include <sys/select.h>
#include <math.h>

#define PI 3.14159265
#define BUFLEN 256
#define DEBUG

const double acc_z_abs_max = 4000.0/415;
int ChooseNum(float push,float x,float y,float z,float the);
int Approx(float a, float  b,float error);

inline double trunc_norm (double val, const double abs_max) {
	if (-abs_max>val || val>abs_max) val = val>0 ? abs_max : -abs_max; //if |val|>abs_max
	//	printf("tttt%lf\n",val);
	return val / abs_max;
}

int openserial (char *sdevfile) {
	int _serial_d = open(sdevfile, O_RDWR | O_NOCTTY);
	if (_serial_d == -1) perror("Unable to open device\n");
	return _serial_d;
}

int main (int argc, char **argv) {
	int i;
	int serial_d;
	speed_t speed;
	struct termios soptions, soptions_org;;
	unsigned char sensor_index = 0+'0'; // indexes are ASCII-based
	char wCommand;
	unsigned char send_buf[BUFLEN];
	unsigned char recv_buf[BUFLEN];
	float acc_x, acc_y, acc_z;
	float data_x, data_y, data_z;
	float data_TSI, theta;
	unsigned int num_recv_c, num_send_c;
	unsigned int timecycle = 0;
	FILE *xdata_fp;
	int file_index = 0; // file extensions: .1, .2,...
	const int sample = 1; // output file will contain 1*sample points
	const char *filebase = "xdata.";
	const int num_file = 2;
	char filename[BUFLEN];

	// no port specified
	if (argc == 1) {
		printf("Usage: zstar3-test sensor_index port: \"zstar3-test 0 /dev/ttyACM0\"\n");
		printf("\"sensor_index\" is used to select sensors if multiple sensors are detected by USB transceiver.\n");
		printf("\"/dev/ttyACM0\" is a USB serial device (cdc_acm kernel module) to support serial control to modems.\n");
		return 1;
	}

	sensor_index = argv[1][0]; // 0~15(F)

	if ((serial_d = openserial(argv[2])) == -1) return 1;

	// ----- Begin of setup serial ports
	tcgetattr(serial_d, &soptions_org);
	tcgetattr(serial_d, &soptions);
	speed = B115200; // Speed options: B19200, B38400, B57600, B115200
	cfsetispeed(&soptions, speed);
	cfsetospeed(&soptions, speed);
	// Enable the reciver and set local mode...
	soptions.c_cflag |= ( CLOCAL | CREAD );
	// Setting Parity Checking (8N1)
	soptions.c_cflag &= ~PARENB;
	soptions.c_cflag &= ~CSTOPB;
	soptions.c_cflag &= ~CSIZE;
	soptions.c_cflag |= CS8;
	// Local setting
	// soptions.c_lflag = (ICANON | ECHO | ECHOE); // canonical
	soptions.c_lflag =  ~(ICANON | ECHO | ECHOE | ISIG); // noncanonical
	// Input setting
	// soptions.c_iflag |= (IXON | IXOFF | IXANY); // software flow control
	soptions.c_iflag |= (INPCK | ISTRIP);
	soptions.c_iflag = IGNPAR;
	// Output setting
	soptions.c_oflag = 0;
	soptions.c_oflag &= ~OPOST;
	// Read options
	soptions.c_cc[VTIME] = 0;
	soptions.c_cc[VMIN] = 1; // transfer at least 1 character or block
	// Apply setting
	tcsetattr(serial_d, TCSANOW, &soptions);
	// ----- End of setup serial ports

	memset(recv_buf, '\0', BUFLEN);
	tcflush(serial_d, TCIOFLUSH);
	usleep(1000);

	printf("Start Measuring...\n");
	while (1) {  //////////////start to capture
		sprintf(filename, "%s%d", filebase, file_index);
		if (xdata_fp = fopen(filename, "w")) {
			fprintf(xdata_fp, "{\"time\":%d,\n\"xarray\":[\n", timecycle);
			for (i=0; i<sample; i++) {
				wCommand = 'V';
				num_send_c = write(serial_d, &wCommand, 1);
				tcdrain(serial_d);

				memset(recv_buf, '\0', BUFLEN); // receive data: Wait for 3-axis ready
				num_recv_c = read(serial_d, &recv_buf[0], BUFLEN);
				tcdrain(serial_d);

				sscanf(recv_buf, " %f %f %f %f,", &data_x, &data_y, &data_z, &data_TSI);
				data_x = data_x/415;
				data_y = data_y/415;
				data_z = data_z/415;
				acc_x = data_x;
				acc_y = data_y;
				acc_z = data_z;
				theta =  acos(trunc_norm(acc_z, acc_z_abs_max)) * 180 / PI;
				int test = ChooseNum(data_TSI,acc_x,acc_y,acc_z,theta);
				switch(test){			       
				case 0 : 
				case 1 : printf("up\n"); break;
				case 2 :
				case 3 : printf("left\n"); break;
				case 4 : 
				case 5 : printf("down\n"); break;
				case 6 :
				case 7 : printf("right\n"); break;
				case 8 : printf("Wrong direction!\n"); break;
				default : printf("break\n"); break;  			
				}
#ifdef DEBUG
				//	printf("X command receive: (%.2f %.2f %.2f %f %f) t=%d\n", data_x, data_y, data_z, theta, data_TSI, timecycle);
#endif

				fprintf(xdata_fp, "[%d],\n", test);

				usleep(1000);
			}
			// Write postscript to file
			fprintf(xdata_fp, "]\n}\n");
			fclose(xdata_fp);
		} else {
			printf("Can not open \"%s\"", filename);
		}
		file_index = (file_index+1) % num_file; // cycle to next file index
		timecycle++; // time counter
	}

	wCommand = 'x'; //Send 'x' to stop burst mode
	num_send_c = write(serial_d, &wCommand, 1);
	tcdrain(serial_d);
	memset(recv_buf, '\0', BUFLEN);
	num_recv_c = read(serial_d, recv_buf, 1);
	tcdrain(serial_d);
	// restore setting and close
	tcsetattr(serial_d, TCSANOW, &soptions_org);
	close(serial_d);

	return 0;
}
int Approx(float a, float b, float error){
	if(fabs(a - b) <= error)
		return 1;
	else 
		return 0;
}
int ChooseNum(float push,float x,float y,float z,float the){
#define t_err 45
	//	#define
/*	if(push>=0 && push<=0.5){     
//push is supposed to be the value of trackpad:0~0.5 is left , 0.5~1 is right , and -1 is no touched
		if(Approx(x,0,3)){
			if( Approx(y,0,3) && Approx(z,9.8,3) && Approx(the,0,t_err))//
				return 1;
			else if( Approx(y,-7,3) && Approx(z,7,3) && Approx(the,45,t_err) )//
				return 2;
			else if( Approx(y,-9.8,3) && Approx(z,0,3) && Approx(the,90,t_err))//
				return 3;
			else if( Approx(y,-7,3) && Approx(z,-7,3) && Approx(the,135,t_err))//
				return 4;
			else if( Approx(y,0,3) && Approx(z,-9.8,3) && Approx(the,180,t_err))//
				return 5;
			else if( Approx(y,7,3) && Approx(z,-7,3) && Approx(the,135,t_err))//
				return 6;
			else if( Approx(y,9.8,3) && Approx(z,0,3) && Approx(the,90,t_err))//
				return 7;
			else if( Approx(y,7,3) && Approx(z,7,3) && Approx(the,45,t_err))//
				return 8;
			else
				return 99;
		}
		else if(Approx(x,9.8,3)){
			return 9;
		}
		else if(Approx(x,-9.8,3)){
			return 0;
		}
		else return 99;
	}
	else if(push>0.5 && push<=1){
		if( Approx(y,0,3) && Approx(z,9.8,3) && Approx(the,0,t_err))//
			return 10;
		else if( Approx(y,-7,3) && Approx(z,7,3) && Approx(the,45,t_err) )//
			return 11;
		else if( Approx(y,-9.8,3) && Approx(z,0,3) && Approx(the,90,t_err))//
			return 12;
		else if( Approx(y,-7,3) && Approx(z,-7,3) && Approx(the,135,t_err))//
			return 13;
		else if( Approx(y,0,3) && Approx(z,-9.8,3) && Approx(the,180,t_err))//
			return 14;
		else if( Approx(y,7,3) && Approx(z,-7,3) && Approx(the,135,t_err))//
			return 15;
		else if( Approx(y,9.8,3) && Approx(z,0,3) && Approx(the,90,t_err))//
			return 16;
		else if( Approx(y,7,3) && Approx(z,7,3) && Approx(the,45,t_err))//
			return 17;
		else
			return 99;
	}
	else
		return 99;*/

	if(push == -1){
		if( Approx(y,0,3) && Approx(z,9.8,3) && Approx(the,0,t_err))//
			return 0;
		else if( Approx(y,-7,3) && Approx(z,7,3) && Approx(the,45,t_err) )//
			return 1;
		else if( Approx(y,-9.8,3) && Approx(z,0,3) && Approx(the,90,t_err))//
			return 2;
		else if( Approx(y,-7,3) && Approx(z,-7,3) && Approx(the,135,t_err))//
			return 3;
		else if( Approx(y,0,3) && Approx(z,-9.8,3) && Approx(the,180,t_err))//
			return 4;
		else if( Approx(y,7,3) && Approx(z,-7,3) && Approx(the,135,t_err))//
			return 5;
		else if( Approx(y,9.8,3) && Approx(z,0,3) && Approx(the,90,t_err))//
			return 6;
		else if( Approx(y,7,3) && Approx(z,7,3) && Approx(the,45,t_err))//
			return 7;
		else
			return 8;
	}
	else return 99;

}
