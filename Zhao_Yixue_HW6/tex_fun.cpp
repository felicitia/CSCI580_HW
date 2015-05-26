/* Texture functions for cs580 GzLib	*/
#include    "stdafx.h" 
#include	"stdio.h"
#include	"Gz.h"

GzColor	*image;
int xs, ys;
int reset = 1;
#define	ARRAY(x,y)	(x+(y*xs))	/* simplify indexing */
typedef struct {
	float r;
	float i;
}  ComplexNumber;

ComplexNumber addComplexNumbers(ComplexNumber a, ComplexNumber b);
ComplexNumber multiplyComplexNumbers(ComplexNumber a, ComplexNumber b);

/* Image texture function */
int tex_fun(float u, float v, GzColor color)
{
  unsigned char		pixel[3];
  unsigned char     dummy;
  char  		foo[8];
  int   		i, j;
  FILE			*fd;

  if (reset) {          /* open and load texture file */
    fd = fopen ("texture", "rb");
    if (fd == NULL) {
      fprintf (stderr, "texture file not found\n");
      exit(-1);
    }
    fscanf (fd, "%s %d %d %c", foo, &xs, &ys, &dummy);
    image = (GzColor*)malloc(sizeof(GzColor)*(xs+1)*(ys+1));
    if (image == NULL) {
      fprintf (stderr, "malloc for texture image failed\n");
      exit(-1);
    }

    for (i = 0; i < xs*ys; i++) {	/* create array of GzColor values */
      fread(pixel, sizeof(pixel), 1, fd);
      image[i][RED] = (float)((int)pixel[RED]) * (1.0 / 255.0);
      image[i][GREEN] = (float)((int)pixel[GREEN]) * (1.0 / 255.0);
      image[i][BLUE] = (float)((int)pixel[BLUE]) * (1.0 / 255.0);
      }

    reset = 0;          /* init is done */
	fclose(fd);
  }

/* bounds-test u,v to make sure nothing will overflow image array bounds */
/* determine texture cell corner values and perform bilinear interpolation */
/* set color to interpolated GzColor value and return */
  GzColor A, B, C, D;
  if (u > 1){
	  u = 1;
  }
  if (u < 0){
	  u = 0;
  }
  if (v > 1){
	  v = 1;
  }
  if (v < 0){
	  v = 0;
  }
  u *= (xs - 1);
  v *= (ys - 1);
  int Ax = floor(u);
  int Ay = floor(v);
  int Bx = ceil(u);
  int By = floor(v);
  int Cx = ceil(u);
  int Cy = ceil(v);
  int Dx = floor(u);
  int Dy = ceil(v);
  // bilinear interpolation
  float s = u - floor(u);
  float t = v - floor(v);
  //get Color at conners
  memcpy(A, image[ARRAY(Ax, Ay)], sizeof(GzColor));
  memcpy(B, image[ARRAY(Bx, By)], sizeof(GzColor));
  memcpy(C, image[ARRAY(Cx, Cy)], sizeof(GzColor));
  memcpy(D, image[ARRAY(Dx, Dy)], sizeof(GzColor));
  //interpolate
  color[RED] = s*t*C[RED] + (1 - s)*t*D[RED] + s*(1 - t)*B[RED] + (1 - s)*(1 - t)*A[RED];
  color[GREEN] = s*t*C[GREEN] + (1 - s)*t*D[GREEN] + s*(1 - t)*B[GREEN] + (1 - s)*(1 - t)*A[GREEN];
  color[BLUE] = s*t*C[BLUE] + (1 - s)*t*D[BLUE] + s*(1 - t)*B[BLUE] + (1 - s)*(1 - t)*A[BLUE];
  return GZ_SUCCESS;
}

/* Procedural texture function */
int ptex_fun(float u, float v, GzColor color)
{
	if (u > 1){
		u = 1;
	}
	if (u < 0){
		u = 0;
	}
	if (v > 1){
		v = 1;
	}
	if (v < 0){
		v = 0;
	}
	ComplexNumber numberC, numberX;
	numberC.r = -0.5;
	numberC.i = 0.6;
	numberX.r = u;
	numberX.i = v;
	int N = 100;
	for (int i = 0; i < N; i++){
		numberX = addComplexNumbers(multiplyComplexNumbers(numberX, numberX), numberC);
		if ((numberX.r*numberX.r + numberX.i*numberX.i)>2){ 
			break;
		}
	}
	float lenX = sqrt(numberX.r*numberX.r + numberX.i*numberX.i);
	if (lenX < 0){
		lenX = 0;
	}
	if (lenX>2){
		lenX = 2;
	}
	GzColor LUT[11];
	//create LUT
	LUT[0][RED] = 1;
	LUT[0][GREEN] = 0;
	LUT[0][BLUE] = 0;
	LUT[1][RED] = 0;
	LUT[1][GREEN] = 1;
	LUT[1][BLUE] = 0;
	LUT[2][RED] = 1;
	LUT[2][GREEN] = 1;
	LUT[2][BLUE] = 0;
	LUT[3][RED] = 0;
	LUT[3][GREEN] = 0;
	LUT[3][BLUE] = 1;
	LUT[4][RED] = 153.0/255;
	LUT[4][GREEN] = 0;
	LUT[4][BLUE] = 1;
	LUT[5][RED] = 1;
	LUT[5][GREEN] = 1;
	LUT[5][BLUE] = 1;
	LUT[6][RED] = 1;
	LUT[6][GREEN] = 153.0 / 255;
	LUT[6][BLUE] = 0;
	LUT[7][RED] = 0;
	LUT[7][GREEN] = 153.0 / 255;
	LUT[7][BLUE] = 204.0 / 255;
	LUT[8][RED] = 204.0 / 255;
	LUT[8][GREEN] = 204.0 / 255;
	LUT[8][BLUE] = 1;
	LUT[9][RED] = 153.0 / 255;
	LUT[9][GREEN] = 102.0 / 255;
	LUT[9][BLUE] = 51.0 / 255;
	LUT[10][RED] = 0;
	LUT[10][GREEN] = 102.0 / 255;
	LUT[10][BLUE] = 0;
	//a=(Si+1 -S)/(Si+1 -Si)	 b=(S-Si)/(Si+1 -Si)   S=lenX
	float Siplus = ceil(lenX * 5)*0.2;
	float Siminus = floor(lenX * 5)*0.2;
	float a = (Siplus - lenX) / (Siplus-Siminus);
	float b = (lenX - Siminus) / (Siplus - Siminus);
	int Ciminus = floor(lenX * 5);
	int Ciplus = ceil(lenX * 5);
	color[RED] = a*LUT[Ciminus][RED] + b*LUT[Ciplus][RED];
	color[GREEN] = a*LUT[Ciminus][GREEN] + b*LUT[Ciplus][GREEN];
	color[BLUE] = a*LUT[Ciminus][BLUE] + b*LUT[Ciplus][BLUE];
	return GZ_SUCCESS;

}

ComplexNumber multiplyComplexNumbers(ComplexNumber a, ComplexNumber b){
	ComplexNumber result;
	result.r = a.r*b.r - a.i*b.i;
	result.i = a.r*b.i + a.i*b.r;
	return result;
}

ComplexNumber addComplexNumbers(ComplexNumber a, ComplexNumber b){
	ComplexNumber result;
	result.r = a.r + b.r;
	result.i = a.i + b.i;
	return result;
}