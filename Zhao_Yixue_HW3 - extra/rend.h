#include "disp.h" /* include your own disp.h file (e.g. hw1)*/

/* Camera defaults */
#define	DEFAULT_FOV		35.0
#define	DEFAULT_IM_Z	(-10.0)  /* world coords for image plane origin */
#define	DEFAULT_IM_Y	(5.0)    /* default look-at point = 0,0,0 */
#define	DEFAULT_IM_X	(-10.0)

#define	DEFAULT_AMBIENT	{0.1, 0.1, 0.1}
#define	DEFAULT_DIFFUSE	{0.7, 0.6, 0.5}
#define	DEFAULT_SPECULAR	{0.2, 0.3, 0.4}
#define	DEFAULT_SPEC		32

#define	MATLEVELS	100		/* how many matrix pushes allowed */
#define	MAX_LIGHTS	10		/* how many lights allowed */
#define MAX_ANIMATION 500	/* how many matirx push in animation stack*/

/* Dummy definition : change it later */
#ifndef GzLight
#define GzLight		GzPointer
#endif

#ifndef GzTexture
#define GzTexture	GzPointer
#endif

#ifndef GZRENDER
#define GZRENDER
typedef struct {			/* define a renderer */
  GzRenderClass	renderClass;
  GzDisplay		*display;
  short		    open;
  GzCamera		camera;		
  short		    matlevel;	        /* top of stack - current xform */
  short			anilevel;
  short			anilevelnow;
  GzMatrix		Ximage[MATLEVELS];	/* stack of xforms (Xsm) */
  GzMatrix		Xnorm[MATLEVELS];	/* xforms for norms (Xim) */
  GzMatrix		XimageAnimation[MAX_ANIMATION];	/*stack of incremental animation xforms */
  GzMatrix		Xsp;		        /* NDC to screen (pers-to-screen) */
  GzColor		flatcolor;          /* color state for flat shaded triangles */
  int			interp_mode;
  int			numlights;
  GzLight		lights[MAX_LIGHTS];
  GzLight		ambientlight;
  GzColor		Ka, Kd, Ks;
  float		    spec;		/* specular power */
  GzTexture		tex_fun;    /* tex_fun(float u, float v, GzColor color) */
}  GzRender;

#define LEFT 0
#define RIGHT 1
#define TOP 2
#define BOTTOM 3

#define PI 3.14159265

#define ANIMATION_FOLDER "animation/"
#define ANIMATION_MAX 500
#define SPACE_MATRIX_NUM 3

typedef struct {			/* define a EdgeEquation */
	float A, B, C;
}  EdgeEquation;

typedef struct{
	float A, B, C, D;
} PlaneEquation;
#endif




// Function declaration
// HW2
int GzNewRender(GzRender **render, GzRenderClass renderClass, GzDisplay *display);
int GzFreeRender(GzRender *render);
int GzBeginRender(GzRender	*render);
int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, 
	GzPointer *valueList);
int GzPutTriangle(GzRender *render, int	numParts, GzToken *nameList,
	GzPointer *valueList);

// HW3
int GzPutCamera(GzRender *render, GzCamera *camera);
int GzPushMatrix(GzRender *render, GzMatrix	matrix);
int GzPopMatrix(GzRender *render);
int pushMatrix2Stack(GzRender* render, GzMatrix matrix);

// Object Translation
int GzRotXMat(float degree, GzMatrix mat);
int GzRotYMat(float degree, GzMatrix mat);
int GzRotZMat(float degree, GzMatrix mat);
int GzTrxMat(GzCoord translate, GzMatrix mat);
int GzScaleMat(GzCoord scale, GzMatrix mat);

short ctoi(float color);
void sortYthenX(GzCoord* vertices, GzCoord v0, GzCoord v1, GzCoord v2);
void sortX(GzCoord* vertices, GzCoord v0, GzCoord v1, GzCoord v2);
void computeCoefficient(EdgeEquation* edge1, EdgeEquation* edge2, EdgeEquation* edge3, GzCoord* vertices, short* edgeFlag);
void flagLeftRight(GzCoord* vertices, short* edgeFlag);
bool flagTopBottom(GzCoord* vertices, short* edgeFlag);
void interpolateZ(GzCoord* vertices, PlaneEquation* plane, short* edgeFlag);
int renderPixelInTriangle(GzRender* render, GzCoord* vertices, PlaneEquation* plane, EdgeEquation* edge1, EdgeEquation* edge2, EdgeEquation* edge3, short* edgeFlag);
float dotProduct(GzCoord a, GzCoord b);
void crossProduct(GzCoord a, GzCoord b, GzCoord result);
int xformToScreenSpace(GzCoord* vertices, GzRender* render);
int matrixProduct(GzMatrix matrix1, float* matrix2, float* result);
bool triOffScreen(GzRender * render, GzCoord * verts);

//animation
int animXformToScreenSpace(GzCoord* vertices, GzRender* render);
void AnimationPutTriangle(GzRender	*render, int numParts, GzToken *nameList,
	GzPointer	*valueList);
void shadeForAnimation(GzCoord norm, GzCoord color);
