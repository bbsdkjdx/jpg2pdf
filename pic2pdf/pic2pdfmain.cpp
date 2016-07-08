#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
//#include <libgen.h>
//#include <getopt.h>
//#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
//#include <sys/time.h>

#ifdef WIN32
#define HAVE_FINDFIRST
#include <io.h>
#else
#define HAVE_GLOB
#include <glob.h>
#endif

#include "Jpeg2PDF.h"

//Gets the JPEG size from the array of data passed to the function, file reference: http://www.obrador.com/essentialjpeg/headerinfo.htm
static int get_jpeg_size(unsigned char* data, unsigned int data_size, unsigned short *width, unsigned short *height, unsigned char *colors, double* dpiX, double* dpiY) {
	//Check for valid JPEG image
	int i=0;   // Keeps track of the position within the file
	if(data[i] == 0xFF && data[i+1] == 0xD8 && data[i+2] == 0xFF ) {
		i += 4;
		// Check for valid JPEG header (null terminated JFIF)
//		if(data[i+2] == 'J' && data[i+3] == 'F' && data[i+4] == 'I' && data[i+5] == 'F' && data[i+6] == 0x00) {
			// Retrieve dpi:
			/* It is also possible to retrieve "rational" dpi from EXIF data -- in that case it'll be really double.
			   Should we prefer EXIF data when present? */
			UINT8 units=data[i+9];
			if(units==1){ // pixels per inch
				*dpiX=data[i+10]*256+data[i+11]; // Xdensity
				*dpiY=data[i+12]*256+data[i+13]; // Ydensity
			}else if(units==2){ // pixels per cm
				*dpiX=(data[i+10]*256+data[i+11])*2.54; // Xdensity converted to dpi
				*dpiY=(data[i+12]*256+data[i+13])*2.54; // Ydensity --> dpi
			}else{ // units==0, fallback to 300dpi? Here EXIF data would be useful.
				*dpiX=*dpiY=300;
			}
			//Retrieve the block length of the first block since the first block will not contain the size of file
			unsigned short block_length = data[i] * 256 + data[i+1];
			while(i<(int)data_size) {
				i+=block_length;               //Increase the file index to get to the next block
				if(i >= (int)data_size) return 0;   //Check to protect against segmentation faults
				if(data[i] != 0xFF) return 0;   //Check that we are truly at the start of another block
				if(data[i+1] >= 0xC0 && data[i+1] <= 0xC2) {            //0xFFC0 is the "Start of frame" marker which contains the file size
					//The structure of the 0xFFC0 block is quite simple [0xFFC0][ushort length][uchar precision][ushort x][ushort y]
					*height = data[i+5]*256 + data[i+6];
					*width = data[i+7]*256 + data[i+8];
					*colors = data[i+9];
					return 1;
				}
				else
				{
					i+=2;                              //Skip the block marker
					block_length = data[i] * 256 + data[i+1];   //Go to the next block
				}
			}
			return 0;                     //If this point is reached then no size was found
//		}else{ return 0; }                  //Not a valid JFIF string
		
	}else{ return 0; }                     //Not a valid SOI header
}

void insertJPEGFile(const char *fileName, int fileSize, PJPEG2PDF pdfId, PageOrientation pageOrientation, ScaleMethod scale, bool cropHeight, bool cropWidth) {
	FILE  *fp;
	unsigned char *jpegBuf;
	int readInSize; 
	unsigned short jpegImgW, jpegImgH;
	unsigned char colors;
	double dpiX, dpiY;

	jpegBuf =(unsigned char*) malloc(fileSize);
	if( jpegBuf==NULL ){
		fprintf(stderr,"Memory allocation error.\n");
		exit(EXIT_FAILURE);
	}

	fp = fopen(fileName, "rb");
	if(fp==NULL){
		fprintf(stderr,"Can't open file '%s'. Aborted.\n", fileName);
		exit(EXIT_FAILURE);
	}
	readInSize = fread(jpegBuf, sizeof(UINT8), fileSize, fp);
	fclose(fp);

	if(readInSize != fileSize) 
		fprintf(stderr,"Warning: File %s should be %d Bytes. But only read in %d Bytes.\n", fileName, fileSize, readInSize);

	if(1 == get_jpeg_size(jpegBuf, readInSize, &jpegImgW, &jpegImgH, &colors, &dpiX, &dpiY)) {
		printf("Adding %s (%dx%d, %.0fx%.0f dpi)\n", fileName, jpegImgW, jpegImgH, dpiX, dpiY);
		/* Add JPEG File into PDF */
		Jpeg2PDF_AddJpeg(pdfId, jpegImgW, jpegImgH, readInSize, jpegBuf, (3==colors), pageOrientation, dpiX, dpiY, scale, cropHeight, cropWidth);
	} else {
		fprintf(stderr,"Can't obtain image dimension from '%s'. Aborted.\n", fileName);
		exit(EXIT_FAILURE);
	}

	free(jpegBuf);
}

void getJpegFileImageDimensions(const char *fileName, int fileSize, double *width, double *height) { //in inches; copy-pasted from insertJPEGFile(), Jpeg2PDF_AddJpeg() call is replaced with width/height calculation.
	FILE  *fp;
	unsigned char *jpegBuf;
	int readInSize; 
	unsigned short jpegImgW, jpegImgH;
	unsigned char colors;
	double dpiX, dpiY;

	jpegBuf = (unsigned char *)malloc(fileSize);
	if( jpegBuf==NULL ){
		fprintf(stderr,"Memory allocation error.\n");
		exit(EXIT_FAILURE);
	}

	fp = fopen(fileName, "rb");
	if(fp==NULL){
		fprintf(stderr,"Can't open file '%s'. Aborted.\n", fileName);
		exit(EXIT_FAILURE);
	}
	readInSize = fread(jpegBuf, sizeof(UINT8), fileSize, fp);
	fclose(fp);

	if(readInSize != fileSize) 
		fprintf(stderr,"Warning: File %s should be %d Bytes. But only read in %d Bytes.\n", fileName, fileSize, readInSize);

	if(1 == get_jpeg_size(jpegBuf, readInSize, &jpegImgW, &jpegImgH, &colors, &dpiX, &dpiY)) {
		//printf("Adding %s (%dx%d, %.0fx%.0f dpi)\n", fileName, jpegImgW, jpegImgH, dpiX, dpiY);
		///* Add JPEG File into PDF */
		//Jpeg2PDF_AddJpeg(pdfId, jpegImgW, jpegImgH, readInSize, jpegBuf, (3==colors), pageOrientation, dpiX, dpiY, scale, cropPage);
		*width=((double)jpegImgW)/dpiX;
		*height=((double)jpegImgH)/dpiY;
	} else {
		fprintf(stderr,"Can't obtain image dimension from '%s'. Aborted.\n", fileName);
		exit(EXIT_FAILURE);
	}

	free(jpegBuf);
}


void findMaximumDimensions(char **filesarray, int globlen, bool fixedOrientation, double *maxWidth, double *maxHeight) {
	int i;
	double width, height;
	struct stat sb;

	*maxWidth=*maxHeight=0;
	for(i=0; i<globlen; i++){
		if (stat(filesarray[i], &sb) == -1) {
			perror("stat");
			exit(EXIT_FAILURE);
		}
		getJpegFileImageDimensions(filesarray[i], sb.st_size, &width, &height);
		if(fixedOrientation){
			if(width>*maxWidth){
				*maxWidth=width;
			}
			if(height>*maxHeight){
				*maxHeight=height;
			}
		}else{ // orientation is unknown, so width is the smallest dimension, and height is the largest dimension (in default portrait orientation)
			if(min(width,height)>*maxWidth){
				*maxWidth=min(width,height);
			}
			if(max(width,height)>*maxHeight){
				*maxHeight=max(width,height);
			}
		}
	}
}


DLLEXP char *_main(char *outputFilename) 
{

	char *title="title", *author="author", *keywords="keywords", *subject="subject", *creator="creator";
	double pageWidth=8.27, pageHeight=11.69, pageMargins=0;
	int globlen=2;
	char **filesarray =NULL;
	char timestamp[40]={0};
	bool paperSizeAuto=false;
	bool cropWidth=false, cropHeight=false;
	PageOrientation pageOrientation=PageOrientationAuto;
	ScaleMethod scale=ScaleFit;

	filesarray = (char **)realloc(filesarray, globlen * sizeof(char *));

	filesarray[0] = "input01.jpg";
	filesarray[1]="input02.jpg";

	if(paperSizeAuto){ // determine paper size from maximum jpeg dimensions
		findMaximumDimensions(filesarray, globlen, pageOrientation==Portrait || pageOrientation==Landscape, &pageWidth, &pageHeight);
	}

	PJPEG2PDF pdfId = Jpeg2PDF_BeginDocument(pageWidth, pageHeight, pageMargins); /* Letter is 8.5x11 inch */

	if(pdfId >= 0) 
	{
		UINT32 pdfSize;
		UINT8  *pdfBuf;
		struct stat sb;
		int idx=0;
		for (idx=0; idx<globlen; idx++) {
			if (stat(filesarray[idx], &sb) == -1) return "stat error.";
			insertJPEGFile(filesarray[idx], sb.st_size, pdfId, pageOrientation, scale, cropHeight, cropWidth);
		}
		
		/* Finalize the PDF and get the PDF Size */
		pdfSize = Jpeg2PDF_EndDocument(pdfId, timestamp, title, author, keywords, subject, creator);
		/* Prepare the PDF Data Buffer based on the PDF Size */
		pdfBuf = (UINT8*)malloc(pdfSize);
		if( pdfBuf==NULL ) return "Memory allocation error.\n";
		/* Get the PDF into the Data Buffer and do the cleanup */
		Jpeg2PDF_GetFinalDocumentAndCleanup(pdfId, pdfBuf, &pdfSize);	/* pdfSize: In: bytes of pdfBuf; Out: bytes used in pdfBuf */
		/* Output the PDF Data Buffer to file */
		FILE *fp = fopen(outputFilename, "wb");
		if( fp==NULL ) return "Can't open file.\n";
		if( fwrite(pdfBuf, sizeof(UINT8), pdfSize, fp) != pdfSize )return "Write error.";
		fclose(fp);
		free(pdfBuf);
		free(filesarray);
	} 
	else 
	{
		return "Error Init.\n";
	}
	return "ok";
}

//int main(int argc, char *argv[]) 
//{
//	printf(_main("all.pdf"));
//}
