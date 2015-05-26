/* CS580 Homework 3 */

#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"

#define LEFT 0
#define RIGHT 1
#define TOP 2
#define BOTTOM 3

#define PI 3.14159265

#define ANIMATION_MAX 500
#define SPACE_MATRIX_NUM 3

typedef struct {			/* define a EdgeEquation */
	float A, B, C;
}  EdgeEquation;

typedef struct{
	float A, B, C, D;
} PlaneEquation;

short ctoi(float color);
void sortYthenX(GzCoord* vertices, GzCoord v0, GzCoord v1, GzCoord v2, GzCoord* normals, GzCoord* sortedNormals);
void sortX(GzCoord* vertices, GzCoord v0, GzCoord v1, GzCoord v2);
void computeCoefficient(EdgeEquation* edge1, EdgeEquation* edge2, EdgeEquation* edge3, GzCoord* vertices, short* edgeFlag);
void flagLeftRight(GzCoord* vertices, short* edgeFlag);
bool flagTopBottom(GzCoord* vertices, short* edgeFlag);
void interpolateZ(GzCoord* vertices, PlaneEquation* plane, short* edgeFlag);
int renderPixelInTriangle(GzRender* render, GzCoord* screenVertices, PlaneEquation* plane, EdgeEquation* edge1, EdgeEquation* edge2, EdgeEquation* edge3, 
	short* edgeFlag, GzCoord* vertices, GzCoord* normals);
float dotProduct(GzCoord a, GzCoord b);
void crossProduct(GzCoord a, GzCoord b, GzCoord result);
int xformToScreenSpace(GzCoord* vertices, GzRender* render, GzCoord* screenVertices);
int matrixProduct(GzMatrix matrix1, float* matrix2, float* result);
bool triOffScreen(GzRender * render, GzCoord * verts);
int xformToImageSpace(GzCoord* coords, GzRender* render, GzCoord* xformCoords);
int normalizeCoord(GzCoord coord);
int calculateVertixColor(GzRender* render, GzCoord imageNormals, GzColor color);
void checkRange01(float &a);

int GzRotXMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along x axis
// Pass back the matrix using mat value
	float radian = degree*PI / 180;
	GzMatrix m = {
		1, 0, 0, 0,
		0, cos(radian), -sin(radian), 0,
		0, sin(radian), cos(radian), 0,
		0, 0, 0, 1
	};
	memcpy(mat, m, sizeof(m));
	return GZ_SUCCESS;
}


int GzRotYMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along y axis
// Pass back the matrix using mat value

	float radian = degree*PI / 180;
	GzMatrix m = {
		cos(radian), 0, sin(radian), 0,
		0, 1, 0, 0,
		-sin(radian), 0, cos(radian), 0,
		0, 0, 0, 1
	};
	memcpy(mat, m, sizeof(m));
	return GZ_SUCCESS;
}


int GzRotZMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along z axis
// Pass back the matrix using mat value

	float radian = degree*PI / 180;
	GzMatrix m = {
		cos(radian), -sin(radian), 0, 0,
		sin(radian), cos(radian), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	memcpy(mat, m, sizeof(m));
	return GZ_SUCCESS;
}


int GzTrxMat(GzCoord translate, GzMatrix mat)
{
// Create translation matrix
// Pass back the matrix using mat value
	GzMatrix m = {
		1, 0, 0, translate[X],
		0, 1, 0, translate[Y],
		0, 0, 1, translate[Z],
		0, 0, 0, 1
	};
	memcpy(mat, m, sizeof(m));
	return GZ_SUCCESS;
}


int GzScaleMat(GzCoord scale, GzMatrix mat)
{
// Create scaling matrix
// Pass back the matrix using mat value
	GzMatrix m = {
		scale[X], 0, 0, 0,
		0, scale[Y], 0, 0,
		0, 0, scale[Z], 0,
		0, 0, 0, 1
	};
	memcpy(mat, m, sizeof(m));
	return GZ_SUCCESS;
}


//----------------------------------------------------------
// Begin main functions

int GzNewRender(GzRender **render, GzRenderClass renderClass, GzDisplay	*display)
{
/*  
- malloc a renderer struct 
- keep closed until all inits are done 
- setup Xsp and anything only done once 
- span interpolator needs pointer to display 
- check for legal class GZ_Z_BUFFER_RENDER 
- init default camera 
*/ 
	
	if (renderClass == NULL || display == NULL){
		AfxMessageBox(_T("renderClass or display is null\n"));
		return GZ_FAILURE;
	}

	*render = new GzRender;
	(*render)->open = 0;
	(*render)->display = display;

	GzCamera camera;
	camera.FOV = DEFAULT_FOV;
	camera.lookat[X] = 0;
	camera.lookat[Y] = 0;
	camera.lookat[Z] = 0;
	camera.position[X] = DEFAULT_IM_X;
	camera.position[Y] = DEFAULT_IM_Y;
	camera.position[Z] = DEFAULT_IM_Z;
	camera.worldup[X] = 0;
	camera.worldup[Y] = 1;
	camera.worldup[Z] = 0;

	(*render)->camera = camera;
	//calculate d
	float d = 1.0 / tan(camera.FOV*PI / 360);
	GzMatrix Xsp = {
		display->xres / 2, 0.0, 0.0, display->xres / 2,
		0.0, -display->yres / 2, 0.0, display->yres / 2,
		0.0, 0.0, INT_MAX / d, 0.0,
		0.0, 0.0, 0.0, 1.0
	};
	memcpy((*render)->Xsp, Xsp, sizeof((*render)->Xsp));

	(*render)->matlevel = -1; //stack empty
	if (renderClass == GZ_Z_BUFFER_RENDER){
		(*render)->renderClass = renderClass;
	}
	else
	{
		AfxMessageBox(_T("renderClass is not GZ_Z_BUFFER_RENDER\n"));
	}

	//set default shade
	(*render)->interp_mode = GZ_RGB_COLOR;
	(*render)->numlights = 0;
	(*render)->spec = DEFAULT_SPEC;
	GzColor Ka = DEFAULT_AMBIENT;
	GzColor Kd = DEFAULT_DIFFUSE;
	GzColor Ks = DEFAULT_SPECULAR;
	(*render)->Ka[RED] = Ka[RED];
	(*render)->Ka[GREEN] = Ka[GREEN];
	(*render)->Ka[BLUE] = Ka[BLUE];
	(*render)->Kd[RED] = Kd[RED];
	(*render)->Kd[GREEN] = Kd[GREEN];
	(*render)->Kd[BLUE] = Kd[BLUE];
	(*render)->Ks[RED] = Ks[RED];
	(*render)->Ks[GREEN] = Ks[GREEN];
	(*render)->Ks[BLUE] = Ks[BLUE];

	return GZ_SUCCESS;

}


int GzFreeRender(GzRender *render)
{
/* 
-free all renderer resources
*/
	if (render == NULL){
		AfxMessageBox(_T("render is null\n"));
		return GZ_FAILURE;
	}
	delete render;
	return GZ_SUCCESS;
}


int GzBeginRender(GzRender *render)
{
/*  
- set up for start of each frame - clear frame buffer 
- compute Xiw and projection xform Xpi from camera definition 
- init Ximage - put Xsp at base of stack, push on Xpi and Xiw 
- now stack contains Xsw and app can push model Xforms if it want to. 
*/ 
	if (render == NULL){
		AfxMessageBox(_T("render is null\n"));
		return GZ_FAILURE;
	}

	//calculate Xpi
	float d = 1 / tan(render->camera.FOV*PI / 360);
	GzMatrix Xpi = {
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 1.0 / d, 1.0
	};

	GzCamera camera = render->camera;
	//up'=up-(up dot product Z)Z
	float zX, zY, zZ, cl_absolute;
	zX = camera.lookat[X] - camera.position[X];
	zY = camera.lookat[Y] - camera.position[Y];
	zZ = camera.lookat[Z] - camera.position[Z];
	cl_absolute = sqrt(pow(zX, 2) + pow(zY, 2) + pow(zZ, 2));
	zX /= cl_absolute;
	zY /= cl_absolute;
	zZ /= cl_absolute;
	GzCoord z = { zX, zY, zZ };
	float coeffient = dotProduct(camera.worldup, z);
	camera.worldup[X] -= (coeffient*z[X]);
	camera.worldup[Y] -= (coeffient*z[Y]);
	camera.worldup[Z] -= (coeffient*z[Z]);
	float newUp_absolute = sqrt(pow(camera.worldup[X], 2) + pow(camera.worldup[Y], 2) + pow(camera.worldup[Z], 2));
	GzCoord y = { camera.worldup[X] / newUp_absolute, camera.worldup[Y] / newUp_absolute, camera.worldup[Z] / newUp_absolute };
	GzCoord x;
	crossProduct(y, z, x);

	GzMatrix Xiw = {
		x[X], x[Y], x[Z], -dotProduct(x, camera.position),
		y[X], y[Y], y[Z], -dotProduct(y, camera.position),
		z[X], z[Y], z[Z], -dotProduct(z, camera.position),
		0.0, 0.0, 0.0, 1.0
	};

	// Push to stack
	GzPushMatrix(render, render->Xsp);
	GzPushMatrix(render, Xpi);
	GzPushMatrix(render, Xiw);

	render->open = 1;
	return GZ_SUCCESS;
}

int GzPutCamera(GzRender *render, GzCamera *camera)
{
/*
- overwrite renderer camera structure with new camera definition
*/

	if (render == NULL || camera == NULL){
		AfxMessageBox(_T("render or camera is null\n"));
		return GZ_FAILURE;
	}
	render->camera = *camera;
	//camera focus changed
	float d = 1.0 / tan(camera->FOV*PI / 360);
	render->Xsp[2][2] = MAXINT / d;
	return GZ_SUCCESS;	
}

int GzPushMatrix(GzRender *render, GzMatrix	matrix)
{
/*
- push a matrix onto the Ximage stack
- check for stack overflow
*/
	
	if (render == NULL || matrix == NULL){
		AfxMessageBox(_T("render or matrix is null\n"));
		return GZ_FAILURE;
	}
	render->matlevel++;
	//valid from 0-99
	if (render->matlevel >= MATLEVELS){
		AfxMessageBox(_T("push a matrix overflow!\n"));
		return GZ_FAILURE;
	}
	if (render->matlevel == 0){
		memcpy(render->Ximage[render->matlevel], matrix, sizeof(GzMatrix));
	}
	else{
		//accumulate matrixs
		GzMatrix currentMatrix = {
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		};
		for (int row = 0; row < 4; row++){
			for (int col = 0; col < 4; col++){
				for (int k = 0; k < 4; k++){
					currentMatrix[row][col] += (render->Ximage[render->matlevel - 1][row][k] * matrix[k][col]);
				}

			}
		}
		memcpy(render->Ximage[render->matlevel], currentMatrix, sizeof(GzMatrix));
	}
	
	//push Matrix to Xnorm Stack
	if (render->matlevel >= 2){
		//strip translations
		matrix[0][3] = 0;
		matrix[1][3] = 0;
		matrix[2][3] = 0;
		//unitary rotation, use any row/col and compute scale factor
		float denominator = sqrt(matrix[0][0] * matrix[0][0] + matrix[1][0] * matrix[1][0] + matrix[2][0] * matrix[2][0]);
		if (denominator == 0){
			AfxMessageBox(_T("denominator is 0\n"));
			return GZ_FAILURE;
		}
		if (denominator != 1){
			float K = 1 / denominator;
			for (int row = 0; row < 3; row++){
				for (int col = 0; col < 3; col++){
					matrix[row][col] *= K;
				}
			}
		}
		
		if (render->matlevel == 2){
			memcpy(render->Xnorm[render->matlevel - 2], matrix, sizeof(GzMatrix));
		}
		else{
			//accumulate matrixs
			GzMatrix currentMatrix = {
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0
			};
			for (int row = 0; row < 4; row++){
				for (int col = 0; col < 4; col++){
					for (int k = 0; k < 4; k++){
						currentMatrix[row][col] += (render->Xnorm[render->matlevel - 3][row][k] * matrix[k][col]);
					}

				}
			}
			memcpy(render->Xnorm[render->matlevel-2], currentMatrix, sizeof(GzMatrix));
		}
	}

	return GZ_SUCCESS;
}

int GzPopMatrix(GzRender *render)
{
/*
- pop a matrix off the Ximage stack
- check for stack underflow
*/
	if (render == NULL ){
		AfxMessageBox(_T("render is null\n"));
		return GZ_FAILURE;
	}
	render->matlevel--;
	//-1 is empty
	if (render->matlevel < -1){
		AfxMessageBox(_T("pop matrix underflow!\n"));
		return GZ_FAILURE;
	}

	return GZ_SUCCESS;
}


int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, 
	GzPointer	*valueList) /* void** valuelist */
{
/*
- set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
- later set shaders, interpolaters, texture maps, and lights
*/
	
	if (render == NULL || nameList==NULL || valueList ==NULL){
		AfxMessageBox(_T("render or namelist or valuelist is null\n"));
		return GZ_FAILURE;
	}
	for (int i = 0; i < numAttributes; i++){
		switch (nameList[i]){
		case GZ_RGB_COLOR:
			memcpy(render->flatcolor, valueList[i], sizeof(GzColor));
			break;
		case GZ_INTERPOLATE:
			memcpy(&render->interp_mode, valueList[i], sizeof(int));
			break;
		case GZ_AMBIENT_LIGHT:
			memcpy(&render->ambientlight, valueList[i], sizeof(GzLight));
			break;
		case GZ_DIRECTIONAL_LIGHT:
			if (render->numlights >= 10){
				AfxMessageBox(_T("no more light can be added\n"));
				return GZ_FAILURE;
			}
			memcpy(&render->lights[render->numlights], valueList[i], sizeof(GzLight));
			render->numlights++;
			break;
		case GZ_AMBIENT_COEFFICIENT:
			memcpy(render->Ka, valueList[i], sizeof(GzColor));
			break;
		case GZ_DIFFUSE_COEFFICIENT:
			memcpy(render->Kd, valueList[i], sizeof(GzColor));
			break;
		case GZ_SPECULAR_COEFFICIENT:
			memcpy(render->Ks, valueList[i], sizeof(GzColor));
			break;
		case GZ_DISTRIBUTION_COEFFICIENT:
			memcpy(&render->spec, valueList[i], sizeof(float));
			break;
		default:
			AfxMessageBox(_T("namelist value is invalid\n"));
			break;
		}
	}
	
	
	return GZ_SUCCESS;
}

int GzPutTriangle(GzRender	*render, int numParts, GzToken *nameList,
	GzPointer	*valueList)
	/* numParts : how many names and values */
{
	/*
	- pass in a triangle description with tokens and values corresponding to
	GZ_POSITION:3 vert positions in model space
	- Xform positions of verts
	- Clip - just discard any triangle with verts behind view plane
	- test for triangles with all three verts off-screen
	- invoke triangle rasterizer
	*/
	if (render == NULL || nameList == NULL || valueList == NULL){
		AfxMessageBox(_T("render or namelist or valuelist is null\n"));
		return GZ_FAILURE;
	}
	
	GzCoord v0, v1, v2;
	GzCoord* vertices = new GzCoord[3];
	GzCoord* screenVertices = new GzCoord[3];
	GzCoord* normals = new GzCoord[3];
	GzCoord* sortedNormals = new GzCoord[3];
	EdgeEquation edge1, edge2, edge3;
	PlaneEquation plane;
	short edgeFlag[3];

	//get attributes from valuelist
	for (int i = 0; i < numParts; i++){
		if (nameList[i] == GZ_NULL_TOKEN){
			continue;
		}
		else if (nameList[i] == GZ_NORMAL){
			//transform normals from model to image space
			normals = (GzCoord*)valueList[i];
		}
		else if (nameList[i] == GZ_POSITION){
			vertices = (GzCoord*)valueList[i];
		}
		else if (nameList[i] == GZ_TEXTURE_INDEX){

		}
	}

	//change x,y,z from world space to screen space and image space
	xformToScreenSpace(vertices, render, screenVertices);
	//check clip, abandon this triangle
	if (screenVertices[0][Z] < 0 || screenVertices[1][Z] < 0 || screenVertices[2][Z] < 0 || triOffScreen(render, screenVertices)){
		return GZ_SUCCESS;
	}

	memcpy(v0, screenVertices[0], sizeof(GzCoord));
	memcpy(v1, screenVertices[1], sizeof(GzCoord));
	memcpy(v2, screenVertices[2], sizeof(GzCoord));

	sortYthenX(screenVertices, v0, v1, v2, normals, sortedNormals);
	if (screenVertices[0][X] == screenVertices[1][Y] && screenVertices[1][Y] == screenVertices[2][Y] ||
		screenVertices[0][X] == screenVertices[1][X] && screenVertices[1][X] == screenVertices[2][X]){
		AfxMessageBox(_T("Triangle is a line\n"));
		return GZ_FAILURE;
	}

	//if no same Y, check L/R
	if (!flagTopBottom(screenVertices, edgeFlag)){
		flagLeftRight(screenVertices, edgeFlag);
	}
	computeCoefficient(&edge1, &edge2, &edge3, screenVertices, edgeFlag);
	interpolateZ(screenVertices, &plane, edgeFlag);
	if (renderPixelInTriangle(render, screenVertices, &plane, &edge1, &edge2, &edge3, edgeFlag, vertices, sortedNormals)){
		return GZ_FAILURE;
	}
	return GZ_SUCCESS;
}

/* NOT part of API - just for general assistance */

short	ctoi(float color)		/* convert float color to GzIntensity short */
{
  return(short)((int)(color * ((1 << 12) - 1)));
}

void sortYthenX(GzCoord* vertices, GzCoord v0, GzCoord v1, GzCoord v2, GzCoord* normals, GzCoord* sortedNormals){
	//sortY first
	if (v0[Y] < v1[Y]){
		if (v1[Y] < v2[Y]){
			memcpy(vertices[0], v0, sizeof(GzCoord));
			memcpy(vertices[1], v1, sizeof(GzCoord));
			memcpy(vertices[2], v2, sizeof(GzCoord));

			memcpy(sortedNormals[0], normals[0], sizeof(GzCoord));
			memcpy(sortedNormals[1], normals[1], sizeof(GzCoord));
			memcpy(sortedNormals[2], normals[2], sizeof(GzCoord));
		}
		else{
			if (v0[Y] < v2[Y]){
				memcpy(vertices[0], v0, sizeof(GzCoord));
				memcpy(vertices[1], v2, sizeof(GzCoord));
				memcpy(vertices[2], v1, sizeof(GzCoord));

				memcpy(sortedNormals[0], normals[0], sizeof(GzCoord));
				memcpy(sortedNormals[1], normals[2], sizeof(GzCoord));
				memcpy(sortedNormals[2], normals[1], sizeof(GzCoord));
			}
			else{
				memcpy(vertices[0], v2, sizeof(GzCoord));
				memcpy(vertices[1], v0, sizeof(GzCoord));
				memcpy(vertices[2], v1, sizeof(GzCoord));

				memcpy(sortedNormals[0], normals[2], sizeof(GzCoord));
				memcpy(sortedNormals[1], normals[0], sizeof(GzCoord));
				memcpy(sortedNormals[2], normals[1], sizeof(GzCoord));
			}
		}
	}
	else{
		if (v0[Y] < v2[Y]){
			memcpy(vertices[0], v1, sizeof(GzCoord));
			memcpy(vertices[1], v0, sizeof(GzCoord));
			memcpy(vertices[2], v2, sizeof(GzCoord));

			memcpy(sortedNormals[0], normals[1], sizeof(GzCoord));
			memcpy(sortedNormals[1], normals[0], sizeof(GzCoord));
			memcpy(sortedNormals[2], normals[2], sizeof(GzCoord));
		}
		else{
			if (v1[Y] < v2[Y]){
				memcpy(vertices[0], v1, sizeof(GzCoord));
				memcpy(vertices[1], v2, sizeof(GzCoord));
				memcpy(vertices[2], v0, sizeof(GzCoord));

				memcpy(sortedNormals[0], normals[1], sizeof(GzCoord));
				memcpy(sortedNormals[1], normals[2], sizeof(GzCoord));
				memcpy(sortedNormals[2], normals[0], sizeof(GzCoord));
			}
			else{
				memcpy(vertices[0], v2, sizeof(GzCoord));
				memcpy(vertices[1], v1, sizeof(GzCoord));
				memcpy(vertices[2], v0, sizeof(GzCoord));

				memcpy(sortedNormals[0], normals[2], sizeof(GzCoord));
				memcpy(sortedNormals[1], normals[1], sizeof(GzCoord));
				memcpy(sortedNormals[2], normals[0], sizeof(GzCoord));
			}
		}
	}
	//sortX
	if (vertices[0][Y] == vertices[1][Y]){
		GzCoord tmpVertix;
		GzCoord tmpNormal;
		if (vertices[0][X] > vertices[1][X]){
			memcpy(tmpVertix, vertices[0], sizeof(GzCoord));
			memcpy(vertices[0], vertices[1], sizeof(GzCoord));
			memcpy(vertices[1], tmpVertix, sizeof(GzCoord));

			memcpy(tmpNormal, sortedNormals[0], sizeof(GzCoord));
			memcpy(sortedNormals[0], sortedNormals[1], sizeof(GzCoord));
			memcpy(sortedNormals[1], tmpNormal, sizeof(GzCoord));
		}
	}
	else if (vertices[1][Y] == vertices[2][Y]){
		GzCoord tmpVertix;
		GzCoord tmpNormal;
		if (vertices[1][X] > vertices[2][X]){
			memcpy(tmpVertix, vertices[1], sizeof(GzCoord));
			memcpy(vertices[1], vertices[2], sizeof(GzCoord));
			memcpy(vertices[2], tmpVertix, sizeof(GzCoord));

			memcpy(tmpNormal, sortedNormals[1], sizeof(GzCoord));
			memcpy(sortedNormals[1], sortedNormals[2], sizeof(GzCoord));
			memcpy(sortedNormals[2], tmpNormal, sizeof(GzCoord));
		}
	}
}

void sortX(GzCoord* vertices, GzCoord v0, GzCoord v1, GzCoord v2){
	if (v0[0] < v1[0]){
		if (v1[0] < v2[0]){
			memcpy(vertices[0], v0, sizeof(GzCoord));
			memcpy(vertices[1], v1, sizeof(GzCoord));
			memcpy(vertices[2], v2, sizeof(GzCoord));
		}
		else{
			if (v0[0] < v2[0]){
				memcpy(vertices[0], v0, sizeof(GzCoord));
				memcpy(vertices[1], v2, sizeof(GzCoord));
				memcpy(vertices[2], v1, sizeof(GzCoord));
			}
			else{
				memcpy(vertices[0], v2, sizeof(GzCoord));
				memcpy(vertices[1], v0, sizeof(GzCoord));
				memcpy(vertices[2], v1, sizeof(GzCoord));
			}
		}
	}
	else{
		if (v0[0] < v2[0]){
			memcpy(vertices[0], v1, sizeof(GzCoord));
			memcpy(vertices[1], v0, sizeof(GzCoord));
			memcpy(vertices[2], v2, sizeof(GzCoord));
		}
		else{
			if (v1[0] < v2[0]){
				memcpy(vertices[0], v1, sizeof(GzCoord));
				memcpy(vertices[1], v2, sizeof(GzCoord));
				memcpy(vertices[2], v0, sizeof(GzCoord));
			}
			else{
				memcpy(vertices[0], v2, sizeof(GzCoord));
				memcpy(vertices[1], v1, sizeof(GzCoord));
				memcpy(vertices[2], v0, sizeof(GzCoord));
			}
		}
	}
}

void computeCoefficient(EdgeEquation* edge1, EdgeEquation* edge2, EdgeEquation* edge3, GzCoord* vertices, short* edgeFlag){
	//A=dY	B=-dX	C=dXY-dYX
	if (LEFT == edgeFlag[0] && LEFT == edgeFlag[1] || TOP == edgeFlag[0] && BOTTOM == edgeFlag[1]){
		edge1->A = vertices[1][Y] - vertices[0][Y];
		edge2->A = vertices[2][Y] - vertices[1][Y];
		edge3->A = vertices[0][Y] - vertices[2][Y];
		edge1->B = vertices[0][X] - vertices[1][X];
		edge2->B = vertices[1][X] - vertices[2][X];
		edge3->B = vertices[2][X] - vertices[0][X];
		edge1->C = -(edge1->B)*vertices[0][Y] - (edge1->A)*vertices[0][X];
		edge2->C = -(edge2->B)*vertices[1][Y] - (edge2->A)*vertices[1][X];
		edge3->C = -(edge3->B)*vertices[2][Y] - (edge3->A)*vertices[2][X];
	}
	else if (LEFT == edgeFlag[0] && RIGHT == edgeFlag[1] || BOTTOM == edgeFlag[0] && BOTTOM == edgeFlag[1]){
		edge1->A = vertices[2][Y] - vertices[0][Y];
		edge2->A = vertices[1][Y] - vertices[2][Y];
		edge3->A = vertices[0][Y] - vertices[1][Y];
		edge1->B = vertices[0][X] - vertices[2][X];
		edge2->B = vertices[2][X] - vertices[1][X];
		edge3->B = vertices[1][X] - vertices[0][X];
		edge1->C = -(edge1->B)*vertices[0][Y] - (edge1->A)*vertices[0][X];
		edge2->C = -(edge2->B)*vertices[2][Y] - (edge2->A)*vertices[2][X];
		edge3->C = -(edge3->B)*vertices[1][Y] - (edge3->A)*vertices[1][X];
	}

}

void flagLeftRight(GzCoord* vertices, short* edgeFlag){
	//(y-y2)(x1-x2) = (x-x2)(y1-y2)
	float x1 = vertices[1][X];
	float x = (vertices[1][Y] - vertices[2][Y])*(vertices[0][X] - vertices[2][X]) / (vertices[0][Y] - vertices[2][Y]) + vertices[2][X];
	if (x1 < x){
		edgeFlag[0] = LEFT;
		edgeFlag[1] = LEFT;
		edgeFlag[2] = RIGHT;
	}
	else{
		edgeFlag[0] = LEFT;
		edgeFlag[1] = RIGHT;
		edgeFlag[2] = RIGHT;
	}
}

/*
check if the triangle has two same Y
true: has same Y
false: no same Y
*/
bool flagTopBottom(GzCoord* vertices, short* edgeFlag){
	if (vertices[0][Y] == vertices[1][Y]){
		edgeFlag[0] = BOTTOM;
		edgeFlag[1] = BOTTOM;
		edgeFlag[2] = TOP;
		return true;
	}
	else if (vertices[1][Y] == vertices[2][Y]){
		edgeFlag[0] = TOP;
		edgeFlag[1] = BOTTOM;
		edgeFlag[2] = TOP;
		return true;
	}
	return false;
}

void interpolateZ(GzCoord* vertices, PlaneEquation* plane, short* edgeFlag){
	float vector1[3];
	float vector2[3];

	if (LEFT == edgeFlag[0] && LEFT == edgeFlag[1] || TOP == edgeFlag[0] && BOTTOM == edgeFlag[1]){
		vector1[X] = vertices[1][X] - vertices[0][X];
		vector1[Y] = vertices[1][Y] - vertices[0][Y];
		vector1[Z] = vertices[1][Z] - vertices[0][Z];
		vector2[X] = vertices[2][X] - vertices[1][X];
		vector2[Y] = vertices[2][Y] - vertices[1][Y];
		vector2[Z] = vertices[2][Z] - vertices[1][Z];
	}
	else if (LEFT == edgeFlag[0] && RIGHT == edgeFlag[1] || BOTTOM == edgeFlag[0] && BOTTOM == edgeFlag[1]){
		vector1[X] = vertices[2][X] - vertices[0][X];
		vector1[Y] = vertices[2][Y] - vertices[0][Y];
		vector1[Z] = vertices[2][Z] - vertices[0][Z];
		vector2[X] = vertices[1][X] - vertices[2][X];
		vector2[Y] = vertices[1][Y] - vertices[2][Y];
		vector2[Z] = vertices[1][Z] - vertices[2][Z];
	}

	plane->A = vector1[Y] * vector2[Z] - vector1[Z] * vector2[Y];
	plane->B = vector1[Z] * vector2[X] - vector1[X] * vector2[Z];
	plane->C = vector1[X] * vector2[Y] - vector1[Y] * vector2[X];
	plane->D = -((plane->A) * vertices[0][X] + (plane->B) * vertices[0][Y] + (plane->C) * vertices[0][Z]);
}

int renderPixelInTriangle(GzRender* render, GzCoord* screenVertices, PlaneEquation* plane, EdgeEquation* edge1, EdgeEquation* edge2, EdgeEquation* edge3, 
	short* edgeFlag, GzCoord* vertices, GzCoord* normals){
	if (plane->C == 0){
		AfxMessageBox(_T("plane C is 0\n"));
		return GZ_FAILURE;
	}
	GzIntensity r, g, b, a;
	a = 1;
	GzCoord* imageVertices = new GzCoord[3];
	GzCoord* imageNormals = new GzCoord[3];
	GzColor* vertixColors = new GzColor[3];
	PlaneEquation redPlane, greenPlane, bluePlane;
	PlaneEquation normXPlane, normYPlane, normZPlane;
	GzColor* tmpVertices = new GzColor[3];//for the use of interpolate : (x,y,r), (x,y,g), (x,y,b), (x,y,normX), (x,y,normY), (x,y,normZ)
	GzColor interpColor;

	if (render->interp_mode != GZ_FLAT){
		xformToImageSpace(vertices, render, imageVertices);
		xformToImageSpace(normals, render, imageNormals);
		
		switch (render->interp_mode)
		{
		case GZ_COLOR: //G Shading
			for (int vertixId = 0; vertixId < 3; vertixId++){
				calculateVertixColor(render, imageNormals[vertixId], vertixColors[vertixId]);
			}
			//RED Equation
			for (int i = 0; i < 3; i++){
				float tmp[3] = { screenVertices[i][X], screenVertices[i][Y], vertixColors[i][RED] };
				memcpy(tmpVertices[i], tmp, sizeof(GzColor));
			}
			interpolateZ(tmpVertices, &redPlane, edgeFlag);
			//GREEN Equation
			for (int i = 0; i < 3; i++){
				float tmp[3] = { screenVertices[i][X], screenVertices[i][Y], vertixColors[i][GREEN] };
				memcpy(tmpVertices[i], tmp, sizeof(GzColor));
			}
			interpolateZ(tmpVertices, &greenPlane, edgeFlag);
			//BLUE Equation
			for (int i = 0; i < 3; i++){
				float tmp[3] = { screenVertices[i][X], screenVertices[i][Y], vertixColors[i][BLUE] };
				memcpy(tmpVertices[i], tmp, sizeof(GzColor));
			}
			interpolateZ(tmpVertices, &bluePlane, edgeFlag);
			break;
		case GZ_NORMALS: //Phong Shading
			//normal X Equation
			for (int i = 0; i < 3; i++){
				float tmp[3] = { screenVertices[i][X], screenVertices[i][Y], imageNormals[i][X] };
				memcpy(tmpVertices[i], tmp, sizeof(GzCoord));
			}
			interpolateZ(tmpVertices, &normXPlane, edgeFlag);
			//normal Y Equation
			for (int i = 0; i < 3; i++){
				float tmp[3] = { screenVertices[i][X], screenVertices[i][Y], imageNormals[i][Y] };
				memcpy(tmpVertices[i], tmp, sizeof(GzCoord));
			}
			interpolateZ(tmpVertices, &normYPlane, edgeFlag);
			//normal Z Equation
			for (int i = 0; i < 3; i++){
				float tmp[3] = { screenVertices[i][X], screenVertices[i][Y], imageNormals[i][Z] };
				memcpy(tmpVertices[i], tmp, sizeof(GzCoord));
			}
			interpolateZ(tmpVertices, &normZPlane, edgeFlag);
			break;
		default:
			AfxMessageBox(_T("invalid interp_mode\n"));
			break;
		}
	}

	int minY = floor(screenVertices[0][Y]);
	int maxY = ceil(screenVertices[2][Y]);
	GzCoord v0, v1, v2;
	memcpy(v0, screenVertices[0], sizeof(GzCoord));
	memcpy(v1, screenVertices[1], sizeof(GzCoord));
	memcpy(v2, screenVertices[2], sizeof(GzCoord));
	sortX(screenVertices, v0, v1, v2);
	int minX = floor(screenVertices[0][X]);
	int maxX = ceil(screenVertices[2][X]);
	if (minX < 0){
		minX = 0;
	}
	if (minY < 0){
		minY = 0;
	}
	if (maxX>render->display->xres - 1){
		maxX = render->display->xres - 1;
	}
	if (maxY>render->display->yres - 1){
		maxY = render->display->yres - 1;
	}
	for (int y = minY; y <= maxY; y++){
		for (int x = minX; x <= maxX; x++){
			float sign1 = edge1->A*(float)x + edge1->B*(float)y + edge1->C;
			float sign2 = edge2->A*(float)x + edge2->B*(float)y + edge2->C;
			float sign3 = edge3->A*(float)x + edge3->B*(float)y + edge3->C;
			//CCW, so only check all positive
			if (sign1 > 0 && sign2 > 0 && sign3 > 0){
				GzDepth zbuf = render->display->fbuf[y*(render->display->xres) + x].z;
				float interpZ = -(plane->D + plane->A*(float)x + plane->B*(float)y) / plane->C;
				float interpColorR, interpColorG, interpColorB, interpNormX, interpNormY, interpNormZ;
				switch (render->interp_mode)
				{
				case GZ_FLAT:
					r = ctoi(render->flatcolor[0]);
					g = ctoi(render->flatcolor[1]);
					b = ctoi(render->flatcolor[2]);
					break;
				case GZ_COLOR:
					interpColorR = -(redPlane.D + redPlane.A*(float)x + redPlane.B*(float)y) / redPlane.C;
					interpColorG = -(greenPlane.D + greenPlane.A*(float)x + greenPlane.B*(float)y) / greenPlane.C;
					interpColorB = -(bluePlane.D + bluePlane.A*(float)x + bluePlane.B*(float)y) / bluePlane.C;
					checkRange01(interpColorR);
					checkRange01(interpColorG);
					checkRange01(interpColorB);
					r = ctoi(interpColorR);
					g = ctoi(interpColorG);
					b = ctoi(interpColorB);
					break;
				case GZ_NORMALS:
					interpNormX = -(normXPlane.D + normXPlane.A*(float)x + normXPlane.B*(float)y) / normXPlane.C;
					interpNormY = -(normYPlane.D + normYPlane.A*(float)x + normYPlane.B*(float)y) / normYPlane.C;
					interpNormZ = -(normZPlane.D + normZPlane.A*(float)x + normZPlane.B*(float)y) / normZPlane.C;
					float tmp[3] = {interpNormX, interpNormY, interpNormZ };
					normalizeCoord(tmp);
					calculateVertixColor(render, tmp, interpColor);
					r = ctoi(interpColor[RED]);
					g = ctoi(interpColor[GREEN]);
					b = ctoi(interpColor[BLUE]);
					break;
				}
				if (interpZ < zbuf){
					GzPutDisplay(render->display, x, y, r, g, b, a, interpZ);
				}
			}
			else if (0 == sign1 && (edgeFlag[0] == LEFT || edgeFlag[0] == TOP) || 0 == sign2 && (edgeFlag[1] == LEFT || edgeFlag[1] == TOP) && 0 == sign3 && (edgeFlag[2] == LEFT || edgeFlag[2] == TOP)){
				//the comment bellow is another way to calculate interpZ, which is so cool! :D
				/*float x1 = vertices[0][X];
				float y1 = vertices[0][Y];
				float z1 = vertices[0][Z];
				float x2 = vertices[1][X];
				float y2 = vertices[1][Y];
				float z2 = vertices[1][Z];
				float x3 = vertices[2][X];
				float y3 = vertices[2][Y];
				float z3 = vertices[2][Z*/
				/*(-(x3 * y2 * z1 - x3 * y1 * z2 - x2 * y3 * z1 + x2 * y1 * z3 + x1 * y3 * z2 - x1 * y2 * z3)
				- x *(y1*z2 - y1*z3 - y2*z1 + y2*z3 + y3*z1 - y3*z2)
				- y *(-x1*z2 + x1 * z3 + x2 * z1 - x2*z3 - x3 * z1 + x3 * z2))
				/ (x1*y2 - x1 * y3 - x2 * y1 + x2 * y3 + x3 * y1 - x3 * y2);*/
				GzDepth zbuf = render->display->fbuf[y*(render->display->xres) + x].z;
				float interpZ = -(plane->D + plane->A*(float)x + plane->B*(float)y) / plane->C;
				float interpColorR, interpColorG, interpColorB, interpNormX, interpNormY, interpNormZ;
				switch (render->interp_mode)
				{
				case GZ_FLAT:
					r = ctoi(render->flatcolor[0]);
					g = ctoi(render->flatcolor[1]);
					b = ctoi(render->flatcolor[2]);
					break;
				case GZ_COLOR:
					interpColorR = -(redPlane.D + redPlane.A*(float)x + redPlane.B*(float)y) / redPlane.C;
					interpColorG = -(greenPlane.D + greenPlane.A*(float)x + greenPlane.B*(float)y) / greenPlane.C;
					interpColorB = -(bluePlane.D + bluePlane.A*(float)x + bluePlane.B*(float)y) / bluePlane.C;
					checkRange01(interpColorR);
					checkRange01(interpColorG);
					checkRange01(interpColorB);
					r = ctoi(interpColorR);
					g = ctoi(interpColorG);
					b = ctoi(interpColorB);
					break;
				case GZ_NORMALS:
					interpNormX = -(normXPlane.D + normXPlane.A*(float)x + normXPlane.B*(float)y) / normXPlane.C;
					interpNormY = -(normYPlane.D + normYPlane.A*(float)x + normYPlane.B*(float)y) / normYPlane.C;
					interpNormZ = -(normZPlane.D + normZPlane.A*(float)x + normZPlane.B*(float)y) / normZPlane.C;
					float tmp[3] = { interpNormX, interpNormY, interpNormZ };
					calculateVertixColor(render, tmp, interpColor);
					r = ctoi(interpColor[RED]);
					g = ctoi(interpColor[GREEN]);
					b = ctoi(interpColor[BLUE]);
					break;
				}
				if (interpZ < zbuf){
					GzPutDisplay(render->display, x, y, r, g, b, a, interpZ);
				}
			}
		}
	}
	return GZ_SUCCESS;
}

float dotProduct(GzCoord a, GzCoord b){
	return (a[0] * b[0] + a[1] * b[1] + a[2] * b[2]);
}

void crossProduct(GzCoord a, GzCoord b, GzCoord result){
	result[X] = a[Y] * b[Z] - a[Z] * b[Y];
	result[Y] = a[Z] * b[X] - a[X] * b[Z];
	result[Z] = a[X] * b[Y] - a[Y] * b[X];
}

int xformToScreenSpace(GzCoord* vertices, GzRender* render, GzCoord* screenVertices){
	if (render == NULL || vertices == NULL){
		AfxMessageBox(_T("vertices or render is NULL!\n"));
		return GZ_FAILURE;
	}
	float vector[4];
	float result[4];
	GzMatrix currentMatrix;
	memcpy(currentMatrix, render->Ximage[render->matlevel], sizeof(GzMatrix));
	for (int vertixId = 0; vertixId < 3; vertixId++){
		vector[X] = vertices[vertixId][X];
		vector[Y] = vertices[vertixId][Y];
		vector[Z] = vertices[vertixId][Z];
		vector[3] = 1.0;
		//transform using currentMatrix
		matrixProduct(currentMatrix, vector, result);
		result[X] /= result[3];
		result[Y] /= result[3];
		result[Z] /= result[3];
		memcpy(&screenVertices[vertixId][X], &result[X], sizeof(float));
		memcpy(&screenVertices[vertixId][Y], &result[Y], sizeof(float));
		memcpy(&screenVertices[vertixId][Z], &result[Z], sizeof(float));
	}
	return GZ_SUCCESS;
}
/**
	matrix product: matrix1(4*4), matrix2(4*1), result(4*1)
**/
int matrixProduct(GzMatrix matrix1, float* matrix2, float* result){
	for (int y = 0; y < 4; y++){
		float value = 0;
		for (int i = 0; i < 4; i++){
			value += (matrix1[y][i] * matrix2[i]);
		}
		result[y] = value;
	}
	return GZ_SUCCESS;
}


int pushMatrix2Stack(GzMatrix* stack, int &top, GzMatrix matrix){
	top++;
	if (top < 0){
		return GZ_FAILURE;
	}
	else if (top == 0){
		memcpy(stack[top], matrix, sizeof(GzMatrix));
	}
	else
	{
		GzMatrix currentMatrix = {
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		};
		for (int row = 0; row < 4; row++){
			for (int col = 0; col < 4; col++){
				for (int k = 0; k < 4; k++){
					currentMatrix[row][col] += (stack[top-1][row][k] * matrix[k][col]);
				}

			}
		}
		memcpy(stack[top], currentMatrix, sizeof(GzMatrix));
	}
	return GZ_SUCCESS;
}

bool triOffScreen(GzRender * render, GzCoord * verts)
{
	if (render == NULL || render->display == NULL || verts == NULL){
		AfxMessageBox(_T("render or vertices is NULL\n"));
		return true; // no image plane is available!
	}
		
	if (verts[0][Y] < 0 && verts[1][Y] < 0 && verts[2][Y] < 0)
		return true;
	else if (verts[0][Y] > render->display->yres && verts[1][Y] > render->display->yres && verts[2][Y] > render->display->yres)
		return true;
	else if (verts[0][X] < 0 && verts[1][X] < 0 && verts[2][X] < 0)
		return true;
	else if (verts[0][X] > render->display->xres && verts[1][X] > render->display->xres && verts[2][X] > render->display->xres)
		return true;
	else
		return false;
}

int xformToImageSpace(GzCoord* coords, GzRender* render, GzCoord* xformCoords){
	if (render == NULL || coords == NULL){
		AfxMessageBox(_T("coords or render is NULL!\n"));
		return GZ_FAILURE;
	}
	float vector[4];
	float result[4];
	GzMatrix currentMatrix;
	memcpy(currentMatrix, render->Xnorm[render->matlevel - 2], sizeof(GzMatrix));
	for (int coordId = 0; coordId < 3; coordId++){
		vector[X] = coords[coordId][X];
		vector[Y] = coords[coordId][Y];
		vector[Z] = coords[coordId][Z];
		vector[3] = 1.0;
		//transform using currentMatrix
		matrixProduct(currentMatrix, vector, result);
		result[X] /= result[3];
		result[Y] /= result[3];
		result[Z] /= result[3];
		memcpy(&xformCoords[coordId][X], &result[X], sizeof(float));
		memcpy(&xformCoords[coordId][Y], &result[Y], sizeof(float));
		memcpy(&xformCoords[coordId][Z], &result[Z], sizeof(float));
	}
	return GZ_SUCCESS;
}

int calculateVertixColor(GzRender* render, GzCoord imageNormal, GzColor color){ 
	// Color = (Ks * sumOverLights[ lightIntensity ( R dot E )^s ] ) + (Kd * sumOverLights[lightIntensity (N dot L)] ) + ( Ka Ia ) 

	GzCoord E = { 0.0, 0.0, -1.0 };
	GzColor sumSpec = { 0, 0, 0 };
	GzColor sumDiffuse = { 0, 0, 0 };
	GzCoord newImageNormal;
	normalizeCoord(imageNormal);
	//add lights
	for (int lightNum = 0; lightNum < render->numlights; lightNum++){
		float dotNL = dotProduct(imageNormal, render->lights[lightNum].direction);
		float dotNE = dotProduct(imageNormal, E);
		//both > 0, compute lighting model
		if (dotNL>0 && dotNE > 0){
			//imageNormal is the same
			memcpy(newImageNormal, imageNormal, sizeof(GzCoord));
		}
		//both <0, flip normal and compute lighting model on backside of surface
		else if (dotNL < 0 && dotNE < 0){
			newImageNormal[X] = -imageNormal[X];
			newImageNormal[Y] = -imageNormal[Y];
			newImageNormal[Z] = -imageNormal[Z];
			//also need to flip dotNL and dotNE
			dotNL *= -1;
			dotNE *= -1;
		}
		//different sign, light and eye on opposite sides of surface so that light contributes zero – skip it
		else{
			continue;
		}
		//calculate R, R = 2(N•L)N - L
		GzCoord R;
		R[X] = 2 * dotNL*newImageNormal[X] - render->lights[lightNum].direction[X];
		R[Y] = 2 * dotNL*newImageNormal[Y] - render->lights[lightNum].direction[Y];
		R[Z] = 2 * dotNL*newImageNormal[Z] - render->lights[lightNum].direction[Z];
		normalizeCoord(R);
		float dotRE = dotProduct(R, E);
		if (dotRE < 0){
			dotRE = 0;
		}
		float tmp = pow(dotRE, render->spec);
		sumSpec[RED] += (tmp*render->lights[lightNum].color[RED]);
		sumSpec[BLUE] += (tmp*render->lights[lightNum].color[BLUE]);
		sumSpec[GREEN] += (tmp*render->lights[lightNum].color[GREEN]);
		
		sumDiffuse[RED] += (dotNL*render->lights[lightNum].color[RED]);
		sumDiffuse[GREEN] += (dotNL*render->lights[lightNum].color[GREEN]);
		sumDiffuse[BLUE] += (dotNL*render->lights[lightNum].color[BLUE]);
	}
	color[RED] = render->Ks[RED] * sumSpec[RED] + render->Kd[RED] * sumDiffuse[RED] + render->Ka[RED] * render->ambientlight.color[RED];
	color[GREEN] = render->Ks[GREEN] * sumSpec[GREEN] + render->Kd[GREEN] * sumDiffuse[GREEN] + render->Ka[GREEN] * render->ambientlight.color[GREEN];
	color[BLUE] = render->Ks[BLUE] * sumSpec[BLUE] + render->Kd[BLUE] * sumDiffuse[BLUE] + render->Ka[BLUE] * render->ambientlight.color[BLUE];
	return GZ_SUCCESS;
}

int normalizeCoord(GzCoord coord){
	float tmp = coord[0] * coord[0] + coord[1] * coord[1] + coord[2] * coord[2];
	if (tmp == 0){
		AfxMessageBox(_T("denomenator is 0!\n"));
		return GZ_FAILURE;
	}
	tmp = sqrt(tmp);
	coord[0] /= tmp;
	coord[1] /= tmp;
	coord[2] /= tmp;
	return GZ_SUCCESS;
}

void checkRange01(float &a){
	if (a < 0){
		a = 0;
	}
	if (a>1){
		a = 1;
	}
}