#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"

#define LEFT 0
#define RIGHT 1
#define TOP 2
#define BOTTOM 3

typedef struct {			/* define a EdgeEquation */
	float A, B, C;
}  EdgeEquation;

typedef struct{
	float A, B, C, D;
} PlaneEquation;

void sortYthenX(GzCoord* vertices, GzCoord v0, GzCoord v1, GzCoord v2);
void sortX(GzCoord* vertices, GzCoord v0, GzCoord v1, GzCoord v2);
void computeCoefficient(EdgeEquation* edge1, EdgeEquation* edge2, EdgeEquation* edge3, GzCoord* vertices, short* edgeFlag);
void flagLeftRight(GzCoord* vertices, short* edgeFlag);
bool flagTopBottom(GzCoord* vertices, short* edgeFlag);
void interpolateZ(GzCoord* vertices, PlaneEquation* plane, short* edgeFlag);
int renderPixelInTriangle(GzRender* render, GzCoord* vertices, PlaneEquation* plane, EdgeEquation* edge1, EdgeEquation* edge2, EdgeEquation* edge3, short* edgeFlag);

int GzNewRender(GzRender **render, GzRenderClass renderClass, GzDisplay *display)
{
	/*
	- malloc a renderer struct
	- keep closed until BeginRender inits are done
	- span interpolator needs pointer to display for pixel writes
	- check for legal class GZ_Z_BUFFER_RENDER
	*/
	if (renderClass == NULL || display == NULL){
		AfxMessageBox(_T("renderClass or display is null\n"));
		return GZ_FAILURE;
	}

	*render = new GzRender;
	(*render)->display = display;
	(*render)->open = 0;
	if (renderClass == GZ_Z_BUFFER_RENDER){
		(*render)->renderClass = renderClass;
	}
	else
	{
		AfxMessageBox(_T("renderClass is not GZ_Z_BUFFER_RENDER\n"));
	}

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


int GzBeginRender(GzRender	*render)
{
	/*
	- set up for start of each frame - init frame buffer
	*/
	if (render == NULL){
		AfxMessageBox(_T("render is null\n"));
		return GZ_FAILURE;
	}
	render->open = 1;
	return GZ_SUCCESS;
}


int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList,
	GzPointer *valueList) /* void** valuelist */
{
	/*
	- set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
	- later set shaders, interpolaters, texture maps, and lights
	*/
	if (render == NULL){
		AfxMessageBox(_T("render is null\n"));
		return GZ_FAILURE;
	}
	for (int i = 0; i < numAttributes; i++){
		if (nameList[i] == GZ_RGB_COLOR){
			memcpy(render->flatcolor, valueList[i], sizeof(render->flatcolor));
		}
	}
	return GZ_SUCCESS;
}


int GzPutTriangle(GzRender *render, int	numParts, GzToken *nameList,
	GzPointer *valueList)
	/* numParts - how many names and values */
{
	/*
	- pass in a triangle description with tokens and values corresponding to
	GZ_NULL_TOKEN:		do nothing - no values
	GZ_POSITION:		3 vert positions in model space
	- Invoke the scan converter and return an error code
	*/
	if (render == NULL){
		AfxMessageBox(_T("render is null\n"));
		return GZ_FAILURE;
	}

	GzCoord v0, v1, v2;
	GzCoord* vertices = new GzCoord[3];
	EdgeEquation edge1, edge2, edge3;
	PlaneEquation plane;
	short edgeFlag[3];

	for (int i = 0; i < numParts; i++){
		if (nameList[i] == GZ_POSITION){
			GzCoord* coords = (GzCoord*)valueList[i];
			v0[X] = coords[0][X];
			v0[Y] = coords[0][Y];
			v0[Z] = coords[0][Z];
			v1[X] = coords[1][X];
			v1[Y] = coords[1][Y];
			v1[Z] = coords[1][Z];
			v2[X] = coords[2][X];
			v2[Y] = coords[2][Y];
			v2[Z] = coords[2][Z];
			//before sort vertices have no values
			sortYthenX(vertices, v0, v1, v2);
			if (vertices[0][X] == vertices[1][Y] && vertices[1][Y] == vertices[2][Y] ||
				vertices[0][X] == vertices[1][X] && vertices[1][X] == vertices[2][X]){
				AfxMessageBox(_T("Triangle is a line\n"));
				return GZ_FAILURE;
			}
			
			//if no same Y, check L/R
			if (!flagTopBottom(vertices, edgeFlag)){
				flagLeftRight(vertices, edgeFlag);
			}	
			computeCoefficient(&edge1, &edge2, &edge3, vertices, edgeFlag);
			interpolateZ(vertices, &plane, edgeFlag);
			if (renderPixelInTriangle(render, vertices, &plane, &edge1, &edge2, &edge3, edgeFlag)){
				return GZ_FAILURE;
			}
		}
	}
	return GZ_SUCCESS;
}

/* NOT part of API - just for general assistance */

short	ctoi(float color)		/* convert float color to GzIntensity short */
{
	return(short)((int)(color * ((1 << 12) - 1)));
}


void sortYthenX(GzCoord* vertices, GzCoord v0, GzCoord v1, GzCoord v2){
	if (v0[Y] < v1[Y]){
		if (v1[Y] < v2[Y]){
			memcpy(vertices[0], v0, sizeof(GzCoord));
			memcpy(vertices[1], v1, sizeof(GzCoord));
			memcpy(vertices[2], v2, sizeof(GzCoord));
		}
		else{
			if (v0[Y] < v2[Y]){
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
		if (v0[Y] < v2[Y]){
			memcpy(vertices[0], v1, sizeof(GzCoord));
			memcpy(vertices[1], v0, sizeof(GzCoord));
			memcpy(vertices[2], v2, sizeof(GzCoord));
		}
		else{
			if (v1[Y] < v2[Y]){
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
	if (vertices[0][Y] == vertices[1][Y]){
		GzCoord tmpVertix;
		if (vertices[0][X] > vertices[1][X]){
			memcpy(tmpVertix, vertices[0], sizeof(GzCoord));
			memcmp(vertices[0], vertices[1], sizeof(GzCoord));
			memcpy(vertices[1], tmpVertix, sizeof(GzCoord));
		}
	}
	else if (vertices[1][Y] == vertices[2][Y]){
		GzCoord tmpVertix;
		if (vertices[1][X] > vertices[2][X]){
			memcpy(tmpVertix, vertices[1], sizeof(GzCoord));
			memcmp(vertices[1], vertices[2], sizeof(GzCoord));
			memcpy(vertices[2], tmpVertix, sizeof(GzCoord));
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

int renderPixelInTriangle(GzRender* render, GzCoord* vertices, PlaneEquation* plane, EdgeEquation* edge1, EdgeEquation* edge2, EdgeEquation* edge3, short* edgeFlag){
	if (plane->C == 0){
		AfxMessageBox(_T("plane C is 0\n"));
		return GZ_FAILURE;
	}
	GzIntensity r = ctoi(render->flatcolor[0]);
	GzIntensity g = ctoi(render->flatcolor[1]);
	GzIntensity b = ctoi(render->flatcolor[2]);
	GzIntensity a = 1;
	int minY = floor(vertices[0][Y]);
	int maxY = ceil(vertices[2][Y]);
	GzCoord v0, v1, v2;
	memcpy(v0, vertices[0], sizeof(GzCoord));
	memcpy(v1, vertices[1], sizeof(GzCoord));
	memcpy(v2, vertices[2], sizeof(GzCoord));
	sortX(vertices, v0, v1, v2);
	int minX = floor(vertices[0][X]);
	int maxX = ceil(vertices[2][X]);
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
				if (interpZ < zbuf){
					GzPutDisplay(render->display, x, y, r, g, b, a, interpZ);
				}
			}
			else if (0 == sign1 && (edgeFlag[0] == LEFT || edgeFlag[0] == TOP) || 0 == sign2 && (edgeFlag[1] == LEFT || edgeFlag[1] == TOP) && 0 == sign3 && (edgeFlag[2] == LEFT || edgeFlag[2] == TOP)){
				GzDepth zbuf = render->display->fbuf[y*(render->display->xres) + x].z;
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
				
				float interpZ = -(plane->D + plane->A*(float)x + plane->B*(float)y) / plane->C;
				if (interpZ < zbuf){
					GzPutDisplay(render->display, x, y, r, g, b, a, interpZ);
				}
			}
		}
	}
	return GZ_SUCCESS;
}