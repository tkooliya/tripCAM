/*
 ============================================================================
 Name        : BBBCAM_OV5642_DigitalCamera.c
 Author      : Lee
 Version     : V1.0
 Copyright   : ArduCAM demo (C)2015 Lee
 Description :
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "UTFT_SPI.h"
#include "BBBCAM.h"

#define BOOL int
#define TRUE 1
#define FALSE 0

#define OV5642_CHIPID_HIGH 0x300a
#define OV5642_CHIPID_LOW 0x300b

void setup()
{
  // i2c pin config
  system("config-pin P9_18 i2c > /dev/null");
  system("config-pin P9_17 i2c > /dev/null");

// SPI pin config
  system("config-pin P9_28 spi_cs > /dev/null");
  system("config-pin P9_29 spi > /dev/null"); // MISO (master in slave out)
  system("config-pin P9_30 spi > /dev/null"); // MOSI (master out slave in)
  system("config-pin P9_31 spi_sclk > /dev/null");


  
  uint8_t vid,pid;
  uint8_t temp; 

  UTFT();
  ArduCAM(OV5642);
  printf("ArduCAM Start!\n");

  //Check if the ArduCAM SPI bus is OK
  write_reg(ARDUCHIP_TEST1, 0x55); //ARDUCHIP_TEST1 = 0x00

  //Change MCU mode
  write_reg(ARDUCHIP_MODE, 0x00);

  //InitLCD();
  
  //Check if the camera module type is OV5642
  rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
  rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
  if((vid != 0x56) || (pid != 0x42))
  	printf("Can't find OV5642 module!\n");
  else
  	printf("OV5642 detected\n");
  	
  //Change to BMP capture mode and initialize the OV5642 module	  	
  set_format(JPEG);

  
  temp = read_reg(ARDUCHIP_TEST1);
  int temp2 = read_reg(0xBE);

  printf("Value of temp in BBBCAM_OV5642_DigitalCamera is %u\n", temp);
  printf("Value of temp in BBBCAM_OV5642_DigitalCamera is %u\n", temp2);
  InitCAM();

  if(temp != 0x55)
  {
	printf("SPI interface Error!\n");
	while(1);
  }
  else{
	printf("SPI interface Good!\n");
	uint8_t bong = read_reg(ARDUCHIP_TRIG);

	uint8_t bing = bong & SHUTTER_MASK;
	printf("The arduchip_trig register is %u, and the if condition is %u\n", bong, bing);
	uint8_t version = read_reg(0x40);
	printf("ARDUCHIP VERSION = %d \n", version);
  }


}

int main(void)
{
	BOOL isShowFlag = TRUE;
	int nmemb = 1;
	int p = 0;
	setup();

	for(p = 0; p < 1; p++){
		uint8_t buf[256];
		static int i = 0;
		static int k = 0;
		static int n = 0;
		uint8_t temp,temp_last;
		uint8_t start_capture = 0;

		//Wait trigger from shutter buttom
		if(read_reg(ARDUCHIP_TRIG))
		{

			printf("Trigger initialized - commencing acquisition \n");
			isShowFlag = FALSE;
			write_reg(ARDUCHIP_MODE, 0x00);
			set_format(JPEG);
			InitCAM();

			write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);		//VSYNC is active HIGH

			//Wait until buttom released
			while(read_reg(ARDUCHIP_TRIG) & SHUTTER_MASK);
			delayms(1000);
			start_capture = 1;
    	
		}
		else
		{
			printf("Entering else statement of read_reg(ARDUCHIP_TRIG) & SHUTTER_MASK");
			if(isShowFlag )
			{
				temp = read_reg(ARDUCHIP_TRIG);
  
				if(!(temp & VSYNC_MASK))				 			//New Frame is coming
				{
					printf("entering the if statement of temp & VSYNC_MASK");
					write_reg(ARDUCHIP_MODE, 0x00);    		//Switch to MCU
					resetXY();
					write_reg(ARDUCHIP_MODE, 0x01);    		//Switch to CAM
					while(!(read_reg(ARDUCHIP_TRIG)&0x01)); 	//Wait for VSYNC is gone
				}
			}
			printf("Exiting the else statement in main!");
		}
		if(start_capture)
		{
			//Flush the FIFO
			flush_fifo();
			//Clear the capture done flag
			clear_fifo_flag();
			//Start capture
			capture();
			printf("Start Capture\n");
			write_reg(ARDUCHIP_TRIG, CAP_DONE_MASK);
		}
  
		if(read_reg(ARDUCHIP_TRIG))
		{

			printf("Capture Done!\n");
    
			//Construct a file name
			memset(filePath,0,20);
			strcat(filePath,"/mnt/remote/myApps");
			getnowtime();
			strcat(filePath,nowtime);
			strcat(filePath,".jpg");
			//Open the new file
			fp = fopen(filePath,"w+");
			if (fp == NULL)
			{
				printf("open file failed");
				return 0;

			}
			i = 0;
			temp = read_fifo();
			//Write first image data to buffer
			buf[i++] = temp;

			//Read JPEG data from FIFO
			while( (temp != 0xD9) | (temp_last != 0xFF) )
			{
				temp_last = temp;
				temp = read_fifo();
				//Write image data to buffer if not full
				if(i < 256)
					buf[i++] = temp;
				else
				{
					//Write 256 uint8_ts image data to file
					fwrite(buf,256,nmemb,fp);
					i = 0;
					buf[i++] = temp;
				}
			}
			//Write the remain uint8_ts in the buffer
			if(i > 0)
				fwrite(buf,i,nmemb,fp);

			//Close the file
			fclose(fp);

			//Clear the capture done flag
			clear_fifo_flag();
			//Clear the start capture flag
			start_capture = 0;
    
			set_format(JPEG);
			InitCAM();
			isShowFlag = TRUE;
			printf("Results Stored!\n");
		}
	}
	return 0;
}
