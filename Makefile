all : OV5642_DigitalCamera

objects = DefaultFonts_SPI.o UTFT_SPI.o BBBCAM.o
CC = arm-linux-gnueabihf-gcc


OV5642_DigitalCamera: $(objects) BBBCAM_OV5642_DigitalCamera.o
	$(CC) -o OV5642_DigitalCamera $(objects) BBBCAM_OV5642_DigitalCamera.o

DefaultFonts_SPI.o: DefaultFonts_SPI.c
	$(CC) -c DefaultFonts_SPI.c

UTFT_SPI.o: UTFT_SPI.c
	$(CC) -c UTFT_SPI.c

BBBCAM.o: BBBCAM.c
	$(CC) -c BBBCAM.c

BBBCAM_OV5642_DigitalCamera.o: BBBCAM_OV5642_DigitalCamera.c
	$(CC) -c BBBCAM_OV5642_DigitalCamera.c

clean:
	rm OV5642_DigitalCamera *.o
	
clean : 
	rm OV2640_Playback OV3640_Playback OV5642_Playback OV7670_Playback OV2640_DigitalCamera OV5642_DigitalCamera 
	rm *.o