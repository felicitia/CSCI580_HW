// CS580HWView.cpp : implementation of the CCS580HWView class
//

#include "stdafx.h"
#include "CS580HW.h"

#include "CS580HWDoc.h"
#include "CS580HWView.h"
#include "RotateDlg.h"
#include "TranslateDlg.h"
#include "ScaleDlg.h"

#include "disp.h"
#include "Application3.h"
#include <string>
#include "conio.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCS580HWView

IMPLEMENT_DYNCREATE(CCS580HWView, CView)

BEGIN_MESSAGE_MAP(CCS580HWView, CView)
	//{{AFX_MSG_MAP(CCS580HWView)
	ON_COMMAND(IDM_RENDER, OnRender)
	ON_COMMAND(IDM_ROTATE, OnRotate)
	ON_COMMAND(IDM_TRANSLATE, OnTranslate)
	ON_COMMAND(IDM_SCALE, OnScale)
	//}}AFX_MSG_MAP
	ON_COMMAND(IDM_ANIMATION, &CCS580HWView::OnAnimation)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCS580HWView construction/destruction

CCS580HWView::CCS580HWView()
{
	// TODO: add construction code here
	m_pApplication = NULL;
	AllocConsole();
}

CCS580HWView::~CCS580HWView()
{
	if(m_pApplication != NULL)
	{
		delete m_pApplication;
	}
}

BOOL CCS580HWView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CCS580HWView drawing

void CCS580HWView::OnDraw(CDC* pDC)
{
	CCS580HWDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
	if(m_pApplication != NULL)
		DrawFrameBuffer(pDC);
}

/////////////////////////////////////////////////////////////////////////////
// CCS580HWView diagnostics

#ifdef _DEBUG
void CCS580HWView::AssertValid() const
{
	CView::AssertValid();
}

void CCS580HWView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCS580HWDoc* CCS580HWView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCS580HWDoc)));
	return (CCS580HWDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCS580HWView message handlers

void CCS580HWView::OnRender() 
{
	// TODO: Add your command handler code here

	// Call renderer 

	// Application 3
	if(m_pApplication != NULL)
		((Application3 *)m_pApplication)->Render();
	else 
		AfxMessageBox(_T("Application was not allocated\n"));

	// Set window size
	CRect clientRect, windowRect;
	int x_offset, y_offset;

	GetClientRect(&clientRect);
	AfxGetMainWnd()->GetWindowRect(&windowRect);
	
	x_offset = windowRect.Width() - clientRect.Width();
	y_offset = windowRect.Height() - clientRect.Height();

	AfxGetMainWnd()->SetWindowPos(NULL, 0, 0, x_offset+m_pApplication->m_nWidth, y_offset+m_pApplication->m_nHeight, NULL/*,SWP_SHOWWINDOW*/);

	Invalidate(true);	
}


void CCS580HWView::DrawFrameBuffer(CDC *pDC)
{
	if(m_pApplication->m_pFrameBuffer == NULL)
    {
        return;
    }

	if(!m_pApplication->m_pRender->open)
	{
		AfxMessageBox(_T("Renderer was not opened\n"));
		return;
	}

    HDC hdc;
    hdc = ::CreateCompatibleDC(pDC->m_hDC);
	HBITMAP m_bitmap;

    // Display the current image
    char buffer[sizeof(BITMAPINFO)];
    BITMAPINFO* binfo = (BITMAPINFO*)buffer;
    memset(binfo, 0, sizeof(BITMAPINFOHEADER));
    binfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    
	// Create the bitmap
    BITMAPINFOHEADER* bih = &binfo->bmiHeader;
	bih->biBitCount = 3*8;
	bih->biWidth = m_pApplication->m_nWidth;
	bih->biHeight = m_pApplication->m_nHeight;
    bih->biPlanes = 1;
    bih->biCompression = BI_RGB;
    bih->biSizeImage = 0;
    
    m_bitmap = CreateDIBSection(hdc, binfo, 0, 0, 0, DIB_RGB_COLORS);

    int colors = DIB_RGB_COLORS;
    
    ::SelectObject(hdc, m_bitmap);
	binfo->bmiHeader.biBitCount = 0;
	GetDIBits(hdc, m_bitmap, 0, 0, 0, binfo, colors);
    binfo->bmiHeader.biBitCount = 24;
    binfo->bmiHeader.biHeight = -abs(binfo->bmiHeader.biHeight);
    SetDIBits(hdc, m_bitmap, 0, m_pApplication->m_nHeight, m_pApplication->m_pFrameBuffer, binfo, colors);

    ::SetStretchBltMode(pDC->m_hDC, COLORONCOLOR);
    CRect client;
    GetClientRect(&client);
    ::BitBlt(pDC->m_hDC, 0, 0, m_pApplication->m_nWidth, m_pApplication->m_nHeight, 
        hdc, 0, 0, SRCCOPY);
    ::DeleteDC(hdc);
    DeleteObject(m_bitmap); 
}

void CCS580HWView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class

	// Assign Application 3
	if(m_pApplication == NULL)
	{
		m_pApplication = new Application3;
	}
	
	// Initialize and begin renderer
	((Application3 *)m_pApplication)->Initialize();
}

// Callback function for rotation  
void CCS580HWView::OnRotate() 
{
	// TODO: Add your command handler code here
	CRotateDlg dlg;
	GzInput* input;
	GzMatrix	rotMat = 
	{ 
		1.0,	0.0,	0.0,	0.0, 
		0.0,	1.0,	0.0,	0.0, 
		0.0,	0.0,	1.0,	0.0, 
		0.0,	0.0,	0.0,	1.0 
	};


	if(m_pApplication == NULL) return;

	input = m_pApplication->m_pUserInput;
	if(input == NULL) return;

	// Initialize
	input->rotation[0] = input->rotation[1] = input->rotation[2] = 0;
	dlg.Initialize(input->rotation[0], input->rotation[1], input->rotation[2]);

	if(dlg.DoModal() == IDOK)
	{
		// Update input rotation value
		input->rotation[dlg.m_nAxis] = dlg.m_fRot;
				
		//  Create Rotation Matrix 
		switch(dlg.m_nAxis)
		{
		case 0 :
			// Create matrix for Rot X
			// for animation
			for (int i = 1; i <= input->rotation[X]; i++){
				GzRotXMat(i, rotMat);
				pushMatrix2Stack(m_pApplication->m_pRender, rotMat);
			}
			// for the render->Ximage
			GzRotXMat(input->rotation[0], rotMat);
			break;
		case 1:
			// for animation
			for (int i = 1; i <= input->rotation[Y]; i++){
				GzRotYMat(i, rotMat);
				pushMatrix2Stack(m_pApplication->m_pRender, rotMat);
			}
			// Create matrix for Rot Y
			GzRotYMat(input->rotation[1], rotMat);
			break;
		case 2:
			// for animation
			for (int i = 1; i <= input->rotation[Z]; i++){
				GzRotZMat(i, rotMat);
				pushMatrix2Stack(m_pApplication->m_pRender, rotMat);
			}
			// Create matrix for Rot Z
			GzRotZMat(input->rotation[2], rotMat);
			break;
		}

		// Accumulate matrix
		GzPushMatrix(m_pApplication->m_pRender, rotMat); 
	}
}

// Callback function for Translation
void CCS580HWView::OnTranslate() 
{
	// TODO: Add your command handler code here
	CTranslateDlg dlg;
	GzInput* input;
	GzMatrix	trxMat = 
	{ 
		1.0,	0.0,	0.0,	0.0, 
		0.0,	1.0,	0.0,	0.0, 
		0.0,	0.0,	1.0,	0.0, 
		0.0,	0.0,	0.0,	1.0 
	};


	if(m_pApplication == NULL) return;

	input = m_pApplication->m_pUserInput;
	if(input == NULL) return;

	// Initialize
	input->translation[0] = input->translation[1] = input->translation[2] = 0;
	dlg.Initialize(input->translation[0], input->translation[1], input->translation[2]);

	if(dlg.DoModal() == IDOK)
	{
		// Update input translation value
		input->translation[0] = dlg.m_fTx; input->translation[1] = dlg.m_fTy; input->translation[2] = dlg.m_fTz;
		//  Create Translation Matrix
		GzTrxMat(input->translation, trxMat);
		// Accumulate matrix
		GzPushMatrix(m_pApplication->m_pRender, trxMat); 
	}
}

// Callback function for Scaling
void CCS580HWView::OnScale() 
{
	// TODO: Add your command handler code here
	CScaleDlg dlg;
	GzInput* input;
	GzMatrix scaleMat = 
	{ 
		1.0,	0.0,	0.0,	0.0, 
		0.0,	1.0,	0.0,	0.0, 
		0.0,	0.0,	1.0,	0.0, 
		0.0,	0.0,	0.0,	1.0 
	};


	if(m_pApplication == NULL) return;

	input = m_pApplication->m_pUserInput;
	if(input == NULL) return;

	// Initialize
	input->scale[0] = input->scale[1] = input->scale[2] = 1;
	dlg.Initialize(input->scale[0], input->scale[1], input->scale[2]);

	if(dlg.DoModal() == IDOK)
	{
		// Update input scale value
		input->scale[0] = dlg.m_fSx; input->scale[1] = dlg.m_fSy; input->scale[2] = dlg.m_fSz;

		//  Create Scaling Matrix
		GzScaleMat(input->scale, scaleMat);

		// Accumulate matrix
		GzPushMatrix(m_pApplication->m_pRender, scaleMat); 
	}
}


void CCS580HWView::OnAnimation()
{
	// TODO: Add your command handler code here

	GzCamera	camera;
	GzToken		nameListTriangle[3]; 	/* vertex attribute names */
	GzPointer	valueListTriangle[3]; 	/* vertex attribute pointers */
	GzToken		nameListColor[3];		/* color type names */
	GzPointer	valueListColor[3];	/* color type rgb pointers */
	GzColor		color;
	GzCoord		vertexList[3];	/* vertex position coordinates */
	GzCoord		normalList[3];	/* vertex normals */
	GzTextureIndex  	uvList[3];		/* vertex texture map indices */
	char		dummy[256];
	int			i, j;
	int			xRes, yRes, dispClass;	/* display parameters */
	int			status;

	
	nameListTriangle[0] = GZ_POSITION;

	/*
	* Walk through the list of triangles, set color
	* and render each triangle
	*/
	FILE *outfile;
	FILE *infile;
	for (; m_pApplication->m_pRender->anilevelnow <= m_pApplication->m_pRender->anilevel; m_pApplication->m_pRender->anilevelnow++){
		GzInitDisplay(m_pApplication->m_pDisplay);
		if ((infile = fopen("pot4.asc", "r")) == NULL)
		{
			AfxMessageBox(_T("The input file was not opened\n"));
		}
		//_cprintf("%s#%d\n", "CS580HWView", m_pApplication->m_pRender->anilevelnow);
		if ((outfile = fopen(("animation/output" + std::to_string(m_pApplication->m_pRender->anilevelnow) + ".ppm").c_str(), "wb")) == NULL)
		{
			AfxMessageBox(_T("The output file was not opened\n"));
		}
		while (fscanf(infile, "%s", dummy) == 1) { 	/* read in tri word */
			fscanf(infile, "%f %f %f %f %f %f %f %f",
				&(vertexList[0][0]), &(vertexList[0][1]),
				&(vertexList[0][2]),
				&(normalList[0][0]), &(normalList[0][1]),
				&(normalList[0][2]),
				&(uvList[0][0]), &(uvList[0][1]));
			fscanf(infile, "%f %f %f %f %f %f %f %f",
				&(vertexList[1][0]), &(vertexList[1][1]),
				&(vertexList[1][2]),
				&(normalList[1][0]), &(normalList[1][1]),
				&(normalList[1][2]),
				&(uvList[1][0]), &(uvList[1][1]));
			fscanf(infile, "%f %f %f %f %f %f %f %f",
				&(vertexList[2][0]), &(vertexList[2][1]),
				&(vertexList[2][2]),
				&(normalList[2][0]), &(normalList[2][1]),
				&(normalList[2][2]),
				&(uvList[2][0]), &(uvList[2][1]));


			//		GzPutAttribute(m_pRender, 1, nameListColor, valueListColor);
			shadeForAnimation(normalList[0], color);/* shade based on the norm of vert0 */
			valueListColor[0] = (GzPointer)color;
			nameListColor[0] = GZ_RGB_COLOR;
			GzPutAttribute(m_pApplication->m_pRender, 1, nameListColor, valueListColor);
			/*
			* Set the value pointers to the first vertex of the
			* triangle, then feed it to the renderer
			*/
			valueListTriangle[0] = (GzPointer)vertexList;
			AnimationPutTriangle(m_pApplication->m_pRender, 1, nameListTriangle, valueListTriangle);
		}
		//_cprintf("%s\n", "write to file");
		GzFlushDisplay2File(outfile, m_pApplication->m_pDisplay); 	/* write out or update display to file*/
		if (fclose(outfile))
			AfxMessageBox(_T("The output file was not closed\n")); 
		if (fclose(infile))
			AfxMessageBox(_T("The input file was not closed\n"));
	}
	AfxMessageBox(_T("YAY! It's done! please check the animation folder! :)\n"));
		
}
