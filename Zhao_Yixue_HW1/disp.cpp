/*   CS580 HW   */
#include    "stdafx.h"  
#include	"Gz.h"
#include	"disp.h"


int GzNewFrameBuffer(char** framebuffer, int width, int height)
{
/* create a framebuffer:
 -- allocate memory for framebuffer : (sizeof)GzPixel x width x height
 -- pass back pointer 
*/
	*framebuffer = new char[3*sizeof(char)*width*height];
	return GZ_SUCCESS;
}

int GzNewDisplay(GzDisplay	**display, GzDisplayClass dispClass, int xRes, int yRes)
{

/* create a display:
  -- allocate memory for indicated class and resolution
  -- pass back pointer to GzDisplay object in display
*/
	if (xRes <= 0 || yRes <= 0){
		AfxMessageBox(_T("The resolution is negative\n"));
		return GZ_FAILURE;
	}
	*display = new GzDisplay;
	int x, y;
	if (xRes > MAXXRES){
		x = MAXXRES;
	}
	else{
		x = xRes;
	}
	if (yRes > MAXYRES){
		y = MAXYRES;
	}
	else{
		y = yRes;
	}
	(*display)->dispClass = dispClass;
	(*display)->xres = x;
	(*display)->yres = y;
	(*display)->fbuf = new GzPixel[x*y];
	return GZ_SUCCESS;
}


int GzFreeDisplay(GzDisplay	*display)
{
/* clean up, free memory */
	if (display == NULL){
		AfxMessageBox(_T("The display is null\n"));
		return GZ_FAILURE;
	}
	delete display;
	return GZ_SUCCESS;
}


int GzGetDisplayParams(GzDisplay *display, int *xRes, int *yRes, GzDisplayClass	*dispClass)
{
/* pass back values for an open display */
	if (display == NULL){
		AfxMessageBox(_T("The display is null\n"));
		return GZ_FAILURE;
	}
	*xRes = display->xres;
	*yRes = display->yres;
	*dispClass = display->dispClass;
	return GZ_SUCCESS;
}


int GzInitDisplay(GzDisplay	*display)
{
/* set everything to some default values - start a new frame */
	if (display == NULL){
		AfxMessageBox(_T("The display is null\n"));
		return GZ_FAILURE;
	}
	for (int i = 0; i < display->xres*display->yres; i++){
		display->fbuf[i].alpha = 1;
		display->fbuf[i].blue = 0.375*MAXINTENSITY;
		display->fbuf[i].green = 0.4375*MAXINTENSITY;
		display->fbuf[i].red = 0.5*MAXINTENSITY;
		display->fbuf[i].z = 0;
	}
	
	return GZ_SUCCESS;
}


int GzPutDisplay(GzDisplay *display, int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
/* write pixel values into the display */
	if (display == NULL){
		AfxMessageBox(_T("The display is null\n"));
		return GZ_FAILURE;
	}
	//negative will duplicate, change Application1.cpp file later
	unsigned short x = display->xres;
	unsigned short y = display->yres;
	if (i < 0) i = 0;
	if (i> x) i = x;
	if (j < 0)j = 0;
	if (j>y) j = y;
	if (r < 0) r = 0;
	if (r>MAXINTENSITY) r = MAXINTENSITY;
	if (g < 0) g = 0;
	if (g>MAXINTENSITY) g = MAXINTENSITY;
	if (b < 0) b = 0;
	if (b>MAXINTENSITY) b = MAXINTENSITY;
	display->fbuf[j*x + i].alpha = a;
	display->fbuf[j*x + i].blue = b;
	display->fbuf[j*x + i].green = g;
	display->fbuf[j*x + i].red = r;
	display->fbuf[j*x + i].z = z;
	return GZ_SUCCESS;
}


int GzGetDisplay(GzDisplay *display, int i, int j, GzIntensity *r, GzIntensity *g, GzIntensity *b, GzIntensity *a, GzDepth *z)
{
	/* pass back pixel value in the display */
	/* check display class to see what vars are valid */

	return GZ_SUCCESS;
}


int GzFlushDisplay2File(FILE* outfile, GzDisplay *display)
{

	/* write pixels to ppm file based on display class -- "P6 %d %d 255\r" */
	if (outfile == NULL){
		AfxMessageBox(_T("The outfile is null\n"));
		return GZ_FAILURE;
	}
	if (display == NULL){
		AfxMessageBox(_T("The display is null\n"));
		return GZ_FAILURE;
	}
	unsigned short x = display->xres;
	unsigned short y = display->yres;
	fprintf(outfile, "%s\n", "P3");
	fprintf(outfile, "%d %d\n", x , y);
	fprintf(outfile,"%d\n", MAXINTENSITY);
	for (int i = 0; i < y; i++){
		for (int j = 0; j < x; j++){
			fprintf(outfile, "%c %c %c ", display->fbuf[i*x+j].red, display->fbuf[i*x+j].green, display->fbuf[i*x+j].blue);
		}
	}
	return GZ_SUCCESS;
}

int GzFlushDisplay2FrameBuffer(char* framebuffer, GzDisplay *display)
{

	/* write pixels to framebuffer: 
		- Put the pixels into the frame buffer
		- Caution: store the pixel to the frame buffer as the order of blue, green, and red 
		- Not red, green, and blue !!!
	*/
	if (framebuffer == NULL || display == NULL){
		AfxMessageBox(_T("The framebuffer or display is null\n"));
		return GZ_FAILURE;
	}
	for (int i = 0; i < display->xres*display->yres; i++){
		framebuffer[i*3] = double(display->fbuf[i].blue)/MAXINTENSITY*255;
		framebuffer[i*3 + 1] = double(display->fbuf[i].green)/MAXINTENSITY*255;
		framebuffer[i*3 + 2] = double(display->fbuf[i].red)/MAXINTENSITY*255;
	}
	
	return GZ_SUCCESS;
}