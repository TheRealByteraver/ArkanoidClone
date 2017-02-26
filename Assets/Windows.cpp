/****************************************************************************************** 
 *	Chili DirectX Framework Version 12.04.24											  *	
 *	Windows.cpp																			  *
 *	Copyright 2012 PlanetChili.net														  *
 *																						  *
 *	This file is part of The Chili DirectX Framework.									  *
 *																						  *
 *	The Chili DirectX Framework is free software: you can redistribute it and/or modify	  *
 *	it under the terms of the GNU General Public License as published by				  *
 *	the Free Software Foundation, either version 3 of the License, or					  *
 *	(at your option) any later version.													  *
 *																						  *
 *	The Chili DirectX Framework is distributed in the hope that it will be useful,		  *
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
 *	GNU General Public License for more details.										  *
 *																						  *
 *	You should have received a copy of the GNU General Public License					  *
 *	along with The Chili DirectX Framework.  If not, see <http://www.gnu.org/licenses/>.  *
 ******************************************************************************************/
#include <Windows.h>
#include <GdiPlus.h>
#include <wchar.h>
#include "Game.h"
#include "resource.h"
#include "Mouse.h"
#include "globaldefs.h"

#pragma comment ( lib,"gdiplus.lib" )

static KeyboardServer kServ;
static MouseServer mServ;
ULONG_PTR gdiplusToken;

LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

		// ************ KEYBOARD MESSAGES ************ //
		case WM_KEYDOWN:
			switch( wParam )
			{
			case VK_UP:
				kServ.OnUpPressed();
				break;
			case VK_DOWN:
				kServ.OnDownPressed();
				break;
			case VK_LEFT:
				kServ.OnLeftPressed();
				break;
			case VK_RIGHT:
				kServ.OnRightPressed();
				break;
			case VK_SPACE:
                kServ.OnSpacePressed();
				break;
			case VK_RETURN:
				kServ.OnEnterPressed();
				break;
			}
			break;
		case WM_KEYUP:
   			switch( wParam )
			{
			case VK_UP:
				kServ.OnUpReleased();
				break;
			case VK_DOWN:
				kServ.OnDownReleased();
				break;
			case VK_LEFT:
				kServ.OnLeftReleased();
				break;
			case VK_RIGHT:
				kServ.OnRightReleased();
				break;
			case VK_SPACE:
				kServ.OnSpaceReleased();
				break;
			case VK_RETURN:
				kServ.OnEnterReleased();
				break;
			}
			break;
		// ************ END KEYBOARD MESSAGES ************ //

		// ************ MOUSE MESSAGES ************ //
		case WM_MOUSEMOVE:
			{
				int x = (short)LOWORD( lParam );
				int y = (short)HIWORD( lParam );

				if( x > 0 && x < HOR_SCREEN_RES && y > 0 && y < VER_SCREEN_RES )
				{
					mServ.OnMouseMove( x,y );
					if( !mServ.IsInWindow() )
					{
						SetCapture( hWnd );
						mServ.OnMouseEnter();
					}
				}
				else
				{
					if( wParam & (MK_LBUTTON | MK_RBUTTON) )
					{
						x = max( 0,x );
						x = min( HOR_SCREEN_RES - 1,x );
						y = max( 0,y );
						y = min( VER_SCREEN_RES - 1,y );
						mServ.OnMouseMove( x,y );
					}
					else
					{
						ReleaseCapture();
						mServ.OnMouseLeave();
						mServ.OnLeftReleased();
						mServ.OnRightReleased();
					}
				}
			}
			break;
		case WM_LBUTTONDOWN:
			mServ.OnLeftPressed();
			break;
		case WM_RBUTTONDOWN:
			mServ.OnRightPressed();
			break;
		case WM_LBUTTONUP:
			mServ.OnLeftReleased();
			break;
		case WM_RBUTTONUP:
			mServ.OnRightReleased();
			break;
		// ************ END MOUSE MESSAGES ************ //
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}


int WINAPI wWinMain( HINSTANCE hInst,HINSTANCE,LPWSTR,INT )
{
	WNDCLASSEX wc = { sizeof( WNDCLASSEX ),CS_CLASSDC,MsgProc,0,0,
                      GetModuleHandle( NULL ),NULL,NULL,NULL,NULL,
                      L"Chili DirectX Framework Window",NULL };
    wc.hIconSm = (HICON)LoadImage( hInst,MAKEINTRESOURCE( IDI_APPICON16 ),IMAGE_ICON,16,16,0 );
	wc.hIcon   = (HICON)LoadImage( hInst,MAKEINTRESOURCE( IDI_APPICON32 ),IMAGE_ICON,32,32,0 );
	wc.hCursor = LoadCursor( NULL,IDC_ARROW );
    RegisterClassEx( &wc );
	
	RECT wr;
	wr.left = 10;
	wr.right = HOR_SCREEN_RES + wr.left;
	wr.top = 30;
	wr.bottom = VER_SCREEN_RES + wr.top;
	AdjustWindowRect( &wr,WS_OVERLAPPEDWINDOW,FALSE );
    HWND hWnd = CreateWindowW( L"Chili DirectX Framework Window",WINDOW_TITLE,
                              WS_OVERLAPPEDWINDOW,wr.left,wr.top,wr.right-wr.left,wr.bottom-wr.top,
                              NULL,NULL,wc.hInstance,NULL );

    ShowWindow( hWnd,SW_SHOWDEFAULT );
    UpdateWindow( hWnd );

	Game theGame( hWnd,kServ,mServ );

    /*
        initialize GDI+
    */
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup( &gdiplusToken,&gdiplusStartupInput,NULL );


    // added mouse confinement code
    RECT rcClip;           // new area for ClipCursor
    RECT rcOldClip;        // previous area for ClipCursor
    // Record the area in which the cursor can move.  
    GetClipCursor(&rcOldClip); 
    // Get the dimensions of the application's window. 
    GetWindowRect(hWnd, &rcClip); 
    /*
    rcClip.left++;
    rcClip.top++;
    rcClip.right--;
    rcClip.bottom--;
    */
    // Confine the cursor to the application's window. 
    //ClipCursor(&rcClip); 
    // end of added mouse confinement code

    MSG msg;
    ZeroMemory( &msg,sizeof( msg ) );
    while( msg.message != WM_QUIT )
    {
        if( PeekMessage( &msg,NULL,0,0,PM_REMOVE ) )
        {
            ClipCursor(&rcOldClip);
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
		{
            // Get the dimensions of the application's window. 
            GetWindowRect(hWnd, &rcClip); 
            // Confine the cursor to the application's window. 
            //ClipCursor(&rcClip); 
            // Process input from the confined cursor. 
            // end of added mouse confinement code
			theGame.Go();
		}
    }
    // added mouse confinement code
    // Restore the cursor to its previous area. 
    ClipCursor(&rcOldClip); 
    // end of added mouse confinement code

    Gdiplus::GdiplusShutdown( gdiplusToken ); // kill GDI+
    UnregisterClass( L"Chili DirectX Framework Window",wc.hInstance );
    return 0;
}