#include "stdafx.h"
#include "Resource.h"

int absmin(int a,int b)
{	if (abs(a)<abs(b)) return a;
	return b;
}

//********************************************************************************
void NewTek_UIThreadIsBusy(void)
{	SetCursor(LoadCursor(g_ControlDLL_hInstance,MAKEINTRESOURCE(IDC_SLEEPING)));
}

//********************************************************************************
void NewTek_DrawBorder(HDC hdc,RECT &rect,byte r,byte g,byte b,bool Pressed)
{	// Get the two colors
	unsigned col1=RGB(min(r+32,255),min(g+32,255),min(b+32,255));
	unsigned col2=RGB(max(r-32,0),max(g-32,0),max(b-32,0));

	// Is the button pressed ?
	if (Pressed) Swap(col1,col2);

	// Top and right
	{	HPEN hPen	=CreatePen(PS_SOLID,1,col1);
		HPEN OldPen	=(HPEN)SelectObject(hdc,hPen);

		// Draw the stuff
		MoveToEx(hdc,rect.right,rect.top,NULL);  
		LineTo  (hdc,rect.left ,rect.top);	
		LineTo  (hdc,rect.left ,rect.bottom);	

		// Delete the stuff
		SelectObject(hdc,OldPen);
		DeleteObject(hPen);
	}

	// Bottom and left
	{	HPEN hPen	=CreatePen(PS_SOLID,1,col2);
		HPEN OldPen	=(HPEN)SelectObject(hdc,hPen);

		// Draw the stuff
		MoveToEx(hdc,rect.right,rect.top,NULL);  
		LineTo  (hdc,rect.right,rect.bottom);	
		LineTo  (hdc,rect.left ,rect.bottom);	

		// Delete the stuff
		SelectObject(hdc,OldPen);
		DeleteObject(hPen);
	}
}

//********************************************************************************
void NewTek_FixRectForScreen(RECT &rect)
{	
	int XRes=GetSystemMetrics(SM_CXSCREEN);
	int YRes=GetSystemMetrics(SM_CYSCREEN);

	if (rect.right >= XRes)
	{	rect.left =XRes-1-(rect.right-rect.left);
		rect.right=XRes-1;
	}

	if (rect.left < 0)
	{	rect.right=rect.right-rect.left;
		rect.left =0;
	}

	if (rect.bottom >= YRes)
	{	rect.top =YRes-1-(rect.bottom-rect.top);
		rect.bottom=YRes-1;
	}

	if (rect.top < 0)
	{	rect.bottom=rect.bottom-rect.top;
		rect.top =0;
	}
}

//********************************************************************************
class Enumeration_X
{	public:		int		X;
				int		YStart;
				int		YEnd;
				int		SnapDistance;
				int		MinDistance;
				HWND	SnappedWith;
				HWND	Parent;
				bool	SnappedToLeft;
				bool	DoNotIncludeChildren;
};

BOOL CALLBACK EnumThreadWndProc_X(HWND hWnd,LPARAM lParam)
{	Enumeration_X *eX=(Enumeration_X*)lParam;

	// If it is not visible ...
	if (!IsWindowVisible(hWnd))		return TRUE;
	if (IsChild(eX->Parent,hWnd))	return TRUE;
	if (hWnd==eX->Parent)			return TRUE;
	
	// Do we snap to this window ?
	RECT rect;
	GetWindowRect(hWnd,&rect);

	// Trivial rejection of this, and all children ... devious speedup !
	if ( (rect.left-eX->SnapDistance>eX->X)||
		 (rect.right+eX->SnapDistance<eX->X)) 
				return TRUE;

	if ( (rect.top-eX->SnapDistance>eX->YEnd)||
		 (rect.bottom+eX->SnapDistance<eX->YStart)) 
				return TRUE;

	// The snapping distance

	// Left side
	if (abs(rect.left - eX->X) < eX->SnapDistance)
	{	eX->MinDistance=absmin(eX->MinDistance,rect.left-eX->X);
		eX->SnappedWith=hWnd;
		eX->SnappedToLeft=true;
	}

	// Right side
	if (abs(rect.right - eX->X) < eX->SnapDistance)
	{	eX->MinDistance=absmin(eX->MinDistance,rect.right-eX->X);
		eX->SnappedWith=hWnd;
		eX->SnappedToLeft=false;
	}

	// Enumerate all child windows of this guy
	if (!eX->DoNotIncludeChildren)
		EnumChildWindows(hWnd,EnumThreadWndProc_X,lParam);

	// Success
	return true;
}

int NewTek_SnapHorizontal(HWND hWnd,int X,int YStart,int YEnd,int SnapDistance,bool DoNotIncludeChildren,HWND *hWndSnappedWith,bool *SnappedToLeft)
{	// Find the root-parent
	Enumeration_X a;
	a.X=X;
	a.YEnd=YEnd;
	a.YStart=YStart;
	a.SnapDistance=SnapDistance;
	a.MinDistance=INT_MAX;
	a.Parent=hWnd;
	a.SnappedWith=NULL;
	a.SnappedToLeft=false;
	a.DoNotIncludeChildren=DoNotIncludeChildren;

	BOOL Res=EnumThreadWindows(	GetCurrentThreadId(),	// thread identifier
								EnumThreadWndProc_X,	// pointer to callback function
								(LPARAM)&a);			// application-defined value

	if (hWndSnappedWith)	*hWndSnappedWith=a.SnappedWith;
	if (SnappedToLeft)		*SnappedToLeft=a.SnappedToLeft;
								
	// Do it !
	return a.MinDistance;
}

//********************************************************************************
class Enumeration_Y
{	public:		int		Y;
				int		XStart;
				int		XEnd;
				int		SnapDistance;
				int		MinDistance;
				HWND	Parent;
				HWND	SnappedWith;
				bool	SnappedToTop;
				bool	DoNotIncludeChildren;
};

BOOL CALLBACK EnumThreadWndProc_Y(HWND hWnd,LPARAM lParam)
{	Enumeration_Y *eY=(Enumeration_Y*)lParam;

	// If it is not visible ...
	if (!IsWindowVisible(hWnd))		return TRUE;
	if (IsChild(eY->Parent,hWnd))	return TRUE;
	if (hWnd==eY->Parent)			return TRUE;
	
	// Do we snap to this window ?
	RECT rect;
	GetWindowRect(hWnd,&rect);

	// Trivial rejection of this, and all children ... devious speedup !
	if ( (rect.top   -eY->SnapDistance>eY->Y)||
		 (rect.bottom+eY->SnapDistance<eY->Y)) 
				return TRUE;

	if ( (rect.left   -eY->SnapDistance>eY->XEnd)||
		 (rect.right  +eY->SnapDistance<eY->XStart)) 
				return TRUE;

	// The snapping distance

	// Left side
	if (abs(rect.top - eY->Y) < eY->SnapDistance)
	{	eY->MinDistance=absmin(eY->MinDistance,rect.top-eY->Y);
		eY->SnappedWith=hWnd;
		eY->SnappedToTop=true;
	}

	// bottom side
	if (abs(rect.bottom - eY->Y) < eY->SnapDistance)
	{	eY->MinDistance=absmin(eY->MinDistance,rect.bottom-eY->Y);
		eY->SnappedWith=hWnd;
		eY->SnappedToTop=false;
	}

	// Enumerate all child windows of this guy
	if (!eY->DoNotIncludeChildren)
		EnumChildWindows(hWnd,EnumThreadWndProc_Y,lParam);

	// Success
	return true;
}

int NewTek_SnapVertical(HWND hWnd,int Y,int XStart,int XEnd,int SnapDistance,bool DoNotIncludeChildren,HWND *hWndSnappedWith,bool *SnappedToTop)
{	// Find the root-parent
	Enumeration_Y a;
	a.Y=Y;
	a.XEnd=XEnd;
	a.XStart=XStart;
	a.SnapDistance=SnapDistance;
	a.MinDistance=INT_MAX;
	a.Parent=hWnd;
	a.SnappedWith=NULL;
	a.SnappedToTop=false;
	a.DoNotIncludeChildren=DoNotIncludeChildren;

	BOOL Res=EnumThreadWindows(	GetCurrentThreadId(),	// thread identifier
								EnumThreadWndProc_Y,	// pointer to callback function
								(LPARAM)&a);			// application-defined value
								
	if (hWndSnappedWith)	*hWndSnappedWith=a.SnappedWith;
	if (SnappedToTop)		*SnappedToTop=a.SnappedToTop;

	// Do it !
	return a.MinDistance;
}

//********************************************************************************
HWND NewTek_GetRootWindow(HWND hWnd)
{	HWND Item=hWnd;
	while(GetParent(Item)) Item=GetParent(Item);

	return Item;
}

//********************************************************************************
void NewTek_PickYUV(int X,int Y,int &y,int &u,int &v)
{	// Get the window
	POINT pt={X,Y},pt2=pt;
	HWND hWnd=WindowFromPoint(pt);

	ScreenToClient(hWnd,&pt2);
	PickYUV *YUV=GetWindowInterface<PickYUV>(hWnd);

	if ((YUV)&&(YUV->PickYUV_Get(pt2.x,pt2.y,y,u,v)))
	{	DynamicGamut DG;
		float r,g,b;
		DG.YUVtoRGB(y,u,v,r,g,b);
	}
	else
	{	HDC hdc=GetDC(NULL);
		WindowPixel a;
		a.bgra=GetPixel(hdc,pt.x,pt.y);
		ReleaseDC(NULL,hdc);
		Swap(a.b,a.r);

		// Convert the color
		DynamicGamut DG;
		float fy,fu,fv;
		
		DG.RGBtoYUV(a.r,a.g,a.b,fy,fu,fv);
		y=fy; u=fu; v=fv;
		float r,g,b;
		DG.YUVtoRGB(fy,fu,fv,r,g,b);
	}
}

//********************************************************************************
int NewTek_fRound(float Val)
{	if (Val>=0) return Val+0.5;
	else		return Val-0.5;
}

//********************************************************************************
void NewTek_DrawRectangleEdge(HDC hdc,long x1,long y1,long x2,long y2,unsigned Width,COLORREF Col,bool Dashed)
{	HPEN	hPen=CreatePen(Dashed?PS_DOT:PS_SOLID,Width,Col);
	HPEN	OldPen=(HPEN)SelectObject(hdc,hPen);
	MoveToEx(hdc,x1,y1,NULL);
	LineTo(hdc,x2,y1);
	LineTo(hdc,x2,y2);
	LineTo(hdc,x1,y2);
	LineTo(hdc,x1,y1);
	SelectObject(hdc,OldPen);
	DeleteObject(hPen);
}

//********************************************************************************
void NewTek_DrawRectangle(	HDC hdc,long x1,long y1,long x2,long y2,
							COLORREF Col)
{	HBRUSH	hBrush=CreateSolidBrush(Col);
	HBRUSH	OldBrush=(HBRUSH)SelectObject(hdc,hBrush);
	HPEN	hPen=CreatePen(PS_SOLID,0,Col);
	HPEN	OldPen=(HPEN)SelectObject(hdc,hPen);
	Rectangle(hdc,x1,y1,x2,y2);
	SelectObject(hdc,OldPen);
	SelectObject(hdc,OldBrush);
	DeleteObject(hBrush);
	DeleteObject(hPen);
}

//********************************************************************************
HWND NewTek_GetPopupParent(HWND Item) {
	do	{
		// Otherwise scan scan though the items until I find a parent
		// that is a popup
		Item=GetParent(Item);
		if (NewTek_IsPopup(Item)) break;
		}
	while(Item);
	return Item;
	}

//********************************************************************************
bool NewTek_IsKeyPressed(int VirtKey)
{	if (GetAsyncKeyState(VirtKey)&((short)(1<<15))) return true;
	return false;
}

//********************************************************************************
void NewTek_PerformLayout(HWND Item)
{	BaseWindowLayoutManager *LayoutManager=GetWindowInterface<BaseWindowLayoutManager>(Item);
	if (LayoutManager) LayoutManager->Layout_PerformLayout(Item);
}

//********************************************************************************
long NewTek_GetWindowPosX(HWND m_hWnd)
{	RECT rect;
	GetWindowRect(m_hWnd,&rect);
	return rect.left;
}

//********************************************************************************
long NewTek_GetWindowPosY(HWND m_hWnd)
{	RECT rect;
	GetWindowRect(m_hWnd,&rect);
	return rect.top;
}

//********************************************************************************
long NewTek_GetWindowHeight(HWND m_hWnd)
{	RECT rect;
	GetClientRect(m_hWnd,&rect);
	return rect.bottom;
}

//********************************************************************************
long NewTek_GetWindowWidth(HWND m_hWnd)
{	RECT rect;
	GetClientRect(m_hWnd,&rect);
	return rect.right;
}

//********************************************************************************
void NewTek_SetWindowSize(HWND m_hWnd,long Width,long Height)
{	SetWindowPos(m_hWnd,NULL,0,0,Width,Height,SWP_NOMOVE|SWP_NOZORDER);

	// If this is not a popup window, perform the layout
	if (!NewTek_IsPopup(m_hWnd)) NewTek_PerformLayout(GetParent(m_hWnd));	
}

//********************************************************************************
void NewTek_SetWindowPosition(HWND m_hWnd,long NewXPosn,long NewYPosn)
{	SetWindowPos(m_hWnd,NULL,NewXPosn,NewYPosn,0,0,SWP_NOSIZE|SWP_NOZORDER);

	// If this is not a popup window, perform the layout
	if (!NewTek_IsPopup(m_hWnd)) NewTek_PerformLayout(GetParent(m_hWnd));	
}

//********************************************************************************
bool NewTek_IsPopup(HWND hWnd)
{	return ( GetWindowLong(hWnd,GWL_STYLE) &WS_POPUP) ? true : false;
}

//********************************************************************************
void NewTek_SetParent(HWND Item,HWND NewParent,bool DoLayouts)
{	// Get the old parent
	HWND Parent=GetParent(Item);

	// Pedantic
	if (NewParent==Parent) return;

	BaseWindowClass *BWC_OldParent=GetWindowInterface<BaseWindowClass>(Parent);
	if (BWC_OldParent) 
	{	BWC_OldParent->Child_AboutToLeave(Item);
		BWC_OldParent->Changed(BaseWindowClass_ChildAboutToLeave,Item,NewParent);
	}	

	// Change it
	SetParent(Item,NewParent);

	// The new parent
	BaseWindowClass *BWC_NewParent=GetWindowInterface<BaseWindowClass>(NewParent);
	if (BWC_NewParent) 
	{	BWC_NewParent->Child_JustEntered(Item);
		BWC_NewParent->Changed(BaseWindowClass_ChildJustEntered,Item,Parent);
	}

	// The item itself
	BaseWindowClass *BWC_Item=GetWindowInterface<BaseWindowClass>(Item);
	if (BWC_Item)
	{	BWC_Item->ParentChanged(Parent,NewParent);
		BWC_Item->Changed(BaseWindowClass_ParentChanged,Parent,NewParent);
	}

	if (DoLayouts)
	{	// ReArrange the old parent window
		if (Parent)		NewTek_PerformLayout(Parent);

		// ReArrange the new parent window
		if (NewParent)	NewTek_PerformLayout(NewParent);
	}
}

//********************************************************************************
RECT NewTek_GetNewChildRect(RECT parent,int xmin,int ymin,int width,int height) {
	long screenwidth=GetSystemMetrics(SM_CXSCREEN);
	long screenheight=GetSystemMetrics(SM_CYSCREEN);
	long childwidth= (width<0) ? xmin : width;
	long childheight= (height<0) ? ymin : height;
	enum placement {unknown,up,down,left,right} selectedposition=unknown;
	long x,y,i,j,t;
	RECT rc;

	//Our first priority is to attempt to place the window to the right or bottom
	if ((x=(screenwidth-(childwidth+parent.right)))>(y=(screenheight-(childheight+parent.bottom)))) {
		if (x>0) selectedposition=right;
		}
	else {
		if (y>0) selectedposition=down;
		}
	//If we can't fit the window to bottom or right, then do plan B
	if (selectedposition==unknown) {
		//Figure the greatest distance from all sides now
		i=(parent.left-childwidth);
		j=(parent.top-childheight);
		//do a linear max sweep x,y,i,j
		selectedposition=right;t=x;
		if (y>t) {selectedposition=down;t=y;}
		if ((i>t)&&(i>0)) {selectedposition=left;t=i;}
		if ((j>t)&&(j>0)) selectedposition=up;
		}
	//At this point we definately have a placement preference
	long parentwidth=parent.right-parent.left;
	long parentheight=parent.bottom-parent.top;
	long idealheight=max(abs(height),ymin);
	long idealwidth=max(abs(width),xmin);

	switch (selectedposition) {
		case up:
			//We can assume that there is enough space to fit the window
			idealheight=min(idealheight,parent.top);
			rc.top=parent.top-idealheight;
			rc.bottom=rc.top+idealheight;
			goto FigureHorizontalForUpandDown;
		case down:
			idealheight=min(idealheight,screenheight-parent.bottom);
			rc.top=parent.bottom;
			rc.bottom=rc.top+idealheight;
			goto FigureHorizontalForUpandDown;
		case left:
			//We can assume that there is enough space to fit the window
			idealwidth=min(idealwidth,parent.left);
			rc.left=parent.left-idealwidth;
			rc.right=rc.left+idealwidth;
			goto FigureVerticalForLeftandRight;
		case right:
			idealwidth=min(idealwidth,screenwidth-parent.right);
			rc.left=parent.right;
			rc.right=rc.left+idealwidth;
			goto FigureVerticalForLeftandRight;

		FigureHorizontalForUpandDown:
			//Figure the Left and Right
			if (width<0) { //is the width resizeable?
				//Can we make the width the same of the parent?
				if (xmin<parentwidth) {//Yes we can
					rc.left=parent.left;
					rc.right=parent.right;
					}
				else { //Nope, obtuse center it with consideration to the screen bounds
					long centerwidth=(xmin-parentwidth)>>1;
					long finalx=max(parent.left-centerwidth,0);
					if (finalx+xmin>screenwidth) finalx=screenwidth-xmin;
					rc.left=finalx;
					rc.right=finalx+xmin;
					}
				}
			else { //Width is not resizeable
				if (width<parentwidth) { //accute center
					long centerwidth=(parentwidth-width)>>1;
					rc.left=parent.left+centerwidth;
					rc.right=rc.left+width;
					}
				else { //obtuse center it with consideration to the screen bounds
					long centerwidth=(width-parentwidth)>>1;
					long finalx=max(parent.left-centerwidth,0);
					if (finalx+width>screenwidth) finalx=screenwidth-width;
					rc.left=finalx;
					rc.right=finalx+width;
					}
				}
			break;

		FigureVerticalForLeftandRight:
			//Figure the Left and Right
			if (height<0) { //is the width resizeable?
				//Can we make the width the same of the parent?
				if (ymin<parentheight) {//Yes we can
					rc.top=parent.top;
					rc.bottom=parent.bottom;
					}
				else { //Nope, obtuse center it with consideration to the screen bounds
					long centerheight=(ymin-parentheight)>>1;
					long finaly=max(parent.top-centerheight,0);
					if (finaly+ymin>screenheight) finaly=screenheight-ymin;
					rc.top=finaly;
					rc.bottom=finaly+ymin;
					}
				}
			else { //Width is not resizeable
				if (height<parentheight) { //accute center
					long centerheight=(parentheight-height)>>1;
					rc.top=parent.top+centerheight;
					rc.bottom=rc.top+height;
					}
				else { //obtuse center it with consideration to the screen bounds
					long centerheight=(height-parentheight)>>1;
					long finaly=max(parent.top-centerheight,0);
					if (finaly+height>screenheight) finaly=screenheight-height;
					rc.top=finaly;
					rc.bottom=finaly+height;
					}
				}
			break;
		}
	return rc;
	}
