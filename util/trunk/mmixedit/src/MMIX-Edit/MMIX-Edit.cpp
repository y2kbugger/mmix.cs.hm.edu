// MMIX-Edit.cpp : Definiert den Einstiegspunkt für die Anwendung.
//

#include "stdafx.h"
#include "MMIX-Edit.h"
#include "SciLexer.h"
#include "Scintilla.h"
#include "FCNTL.H"
#include <Windows.h>
#include <sstream>
#include <ios>
#include <stdio.h>
#include <io.h>

// For nice controls look 
#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' "\
  "name='Microsoft.Windows.Common-Controls' "\
  "version='6.0.0.0' "\
  "processorArchitecture='*' "\
  "publicKeyToken='6595b64144ccf1df' "\
  "language='*'\"")
 
#pragma comment(lib, "ComCtl32.lib")

#define MAX_LOADSTRING 100
#define ErrorMarker 3
#define BackgroundMarker 4

// Globale Variablen:
HINSTANCE hInst;								// Aktuelle Instanz
TCHAR szTitle[MAX_LOADSTRING];					// Titelleistentext
TCHAR szWindowClass[MAX_LOADSTRING];			// Klassenname des Hauptfensters

// Vorwärtsdeklarationen der in diesem Codemodul enthaltenen Funktionen:
ATOM				MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

const wchar_t appName[] = L"MMIX-Edit";
const int blockSize = 128 * 1024;

const COLORREF black = RGB(0,0,0);
const COLORREF white = RGB(0xff,0xff,0xff);
const COLORREF nearlyWhite = RGB(0xF0, 0xF0, 0xF0);

wchar_t home[256];

struct DMApp {
	HINSTANCE hInstance;
	HWND currentDialog;
	HWND wMain;
	HWND wInner;
	HWND wErrorList;
	HWND wEditor;
	bool isDirty;
	wchar_t fullPath[MAX_PATH];
	FILE* currentFile;

	DMApp();

	LRESULT SendEditor(UINT Msg, WPARAM wParam=0, LPARAM lParam=0) {
		return ::SendMessage(wEditor, Msg, wParam, lParam);
	}

	void GetRange(int start, int end, char *text);

	void SetTitle();
	void New();
	void Open();
	void OpenFile(const wchar_t *fileName);
	void Save();
	void SaveAs();
	void SaveFile(const wchar_t *fileName);
	int SaveIfUnsure();

	void Command(int id);
	void EnableAMenuItem(int id, bool enable);
	void CheckMenus();
	void Notify(SCNotification *notification);
	void positionCaret();

	void SetAStyle(int style, COLORREF fore, COLORREF back=white, int size=-1, const char *face=0);
	void InitialiseEditor();
};

static DMApp app;

DMApp::DMApp() {
	hInstance = 0;
	currentDialog = 0;
	wMain = 0;
	wEditor = 0;
	isDirty = false;
	fullPath[0] = '\0';
	currentFile = NULL;
}

void DMApp::GetRange(int start, int end, char *text) {
	TEXTRANGE tr;
	tr.chrg.cpMin = start;
	tr.chrg.cpMax = end;
	tr.lpstrText = (LPWSTR) text;
	SendMessage(wEditor, SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));
}

void DMApp::SetTitle() {
	wchar_t title[MAX_PATH + 100];
	swprintf(title, appName, NULL);
	wcscat(title, L" - ");
	wcscat(title, fullPath);
	::SetWindowText(wMain, title);
}

void DMApp::New() {
	app.SendEditor(SCI_MARKERDELETEALL, -1);
	SendMessage(app.wErrorList, LB_RESETCONTENT, 0, 0);
	SaveIfUnsure();
	SendEditor(SCI_CLEARALL);
	SendEditor(EM_EMPTYUNDOBUFFER);
	swprintf(fullPath, L"\\New.mms\0");
	SetTitle();
	isDirty = false;
	SendEditor(SCI_SETSAVEPOINT);
}

void DMApp::OpenFile(const wchar_t *fileName) {
	
	app.SendEditor(SCI_MARKERDELETEALL, -1);
	SendMessage(app.wErrorList, LB_RESETCONTENT, 0, 0);

	New();
	SendEditor(SCI_CANCEL);
	SendEditor(SCI_SETUNDOCOLLECTION, 0);

	swprintf(fullPath, fileName, NULL);
	currentFile = _wfopen(fullPath, L"rb");
	if (currentFile) {
		SetTitle();
		char data[blockSize];
		int lenFile = fread(data, 1, sizeof(data), currentFile);
		while (lenFile > 0) {
			SendEditor(SCI_ADDTEXT, lenFile,
					   reinterpret_cast<LPARAM>(static_cast<char *>(data)));
			lenFile = fread(data, 1, sizeof(data), currentFile);
		}
		fclose(currentFile);
	} else {
		wchar_t msg[MAX_PATH + 100];
		swprintf(msg, L"Could not open file \"", NULL);
		wcscat(msg, fullPath);
		wcscat(msg, L"\".");
		::MessageBox(wMain, (LPWSTR) msg, (LPWSTR) appName, MB_OK);
	}
	SendEditor(SCI_SETUNDOCOLLECTION, 1);
	::SetFocus(wEditor);
	SendEditor(EM_EMPTYUNDOBUFFER);
	SendEditor(SCI_SETSAVEPOINT);
	SendEditor(SCI_GOTOPOS, 0);
}

void DMApp::Open() {
	
	app.SendEditor(SCI_MARKERDELETEALL, -1);
	SendMessage(app.wErrorList, LB_RESETCONTENT, 0, 0);

	wchar_t openName[MAX_PATH] = L"\0";
	OPENFILENAME ofn = {sizeof(OPENFILENAME)};
	ofn.hwndOwner = wMain;
	ofn.hInstance = hInstance;
	ofn.lpstrFile = (LPWSTR) openName;
	ofn.nMaxFile = sizeof(openName);
	wchar_t *filter = 
		L"MMIX (.mms)\0*.mms\0"
		L"All Files (*.*)\0*.*\0\0";

	ofn.lpstrFilter = (LPWSTR) filter;
	ofn.lpstrCustomFilter = 0;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrTitle = L"Open File";
	ofn.Flags = OFN_HIDEREADONLY;

	if (::GetOpenFileName(&ofn)) {
		OpenFile(openName);
	}

	char buffer[7];
	sprintf(buffer, "%s", "_");
	sprintf(buffer, "%d", (int)SendEditor(SCI_GETLINECOUNT));

	SendEditor(SCI_SETMARGINWIDTHN, 0, (int)SendEditor(SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)buffer) + 5);
}

void DMApp::Save() {
	SaveFile(fullPath);
}

void DMApp::SaveAs() {
	wchar_t openName[MAX_PATH] = L"\0";
	swprintf(openName, fullPath, NULL);
	OPENFILENAME ofn = {sizeof(ofn)};
	ofn.hwndOwner = wMain;
	ofn.hInstance = hInstance;
	ofn.lpstrFile = (LPWSTR) openName;
	ofn.nMaxFile = sizeof(openName);
	ofn.lpstrTitle = L"Save File";
	ofn.Flags = OFN_HIDEREADONLY;

	if (::GetSaveFileName(&ofn)) {
		swprintf(fullPath, openName, NULL);
		SetTitle();
		SaveFile(fullPath);
		::InvalidateRect(wEditor, 0, 0);
	}
}

void DMApp::SaveFile(const wchar_t *fileName) {
	currentFile = _wfopen(fullPath, L"wb");
	if (currentFile) {
		char data[blockSize + 1];
		int lengthDoc = SendEditor(SCI_GETLENGTH);
		for (int i = 0; i < lengthDoc; i += blockSize) {
			int grabSize = lengthDoc - i;
			if (grabSize > blockSize)
				grabSize = blockSize;
			GetRange(i, i + grabSize, data);
			fwrite(data, grabSize, 1, currentFile);
		}
		fclose(currentFile);
		SendEditor(SCI_SETSAVEPOINT);
	} else {
		wchar_t msg[MAX_PATH + 100];
		swprintf(msg, L"Could not save file \"", NULL);
		wcscat(msg, fullPath);
		wcscat(msg, L"\".");
		MessageBox(wMain, (LPWSTR) msg, (LPWSTR) appName, MB_OK);
	}
}

int DMApp::SaveIfUnsure() {
	if (isDirty) {
		wchar_t msg[MAX_PATH + 100];
		swprintf(msg, L"Save changes to \"", NULL);
		wcscat(msg, fullPath);
		wcscat(msg, L"\"?");
		int decision = MessageBox(wMain, msg, (LPCWSTR) appName, MB_YESNOCANCEL);
		if (decision == IDYES) {
			Save();
		}
		return decision;
	}
	return IDYES;
}


void DMApp::Command(int id) {
	switch (id) {

	case IDM_FILE_NEW:
		if (SaveIfUnsure() != IDCANCEL) {
			New();
		}
		break;
	case IDM_FILE_OPEN:
		if (SaveIfUnsure() != IDCANCEL) {
			Open();
		}
		break;
	case IDM_FILE_SAVE:
		Save();
		break;
	case IDM_FILE_SAVEAS:
		SaveAs();
		break;
	case IDM_FILE_EXIT:
		if (SaveIfUnsure() != IDCANCEL) {
			::PostQuitMessage(0);
		}
		break;

	case IDM_EDIT_UNDO:
		SendEditor(WM_UNDO);
		break;
	case IDM_EDIT_REDO:
		SendEditor(SCI_REDO);
		break;
	case IDM_EDIT_CUT:
		SendEditor(WM_CUT);
		break;
	case IDM_EDIT_COPY:
		SendEditor(WM_COPY);
		break;
	case IDM_EDIT_PASTE:
		SendEditor(WM_PASTE);
		break;
	case IDM_EDIT_DELETE:
		SendEditor(WM_CLEAR);
		break;
	case IDM_EDIT_SELECTALL:
		SendEditor(SCI_SELECTALL);
		break;
	};
}

void DMApp::EnableAMenuItem(int id, bool enable) {
	if (enable)
		::EnableMenuItem(::GetMenu(wMain), id, MF_ENABLED | MF_BYCOMMAND);
	else
		::EnableMenuItem(::GetMenu(wMain), id, MF_DISABLED | MF_GRAYED | MF_BYCOMMAND);
}

void DMApp::CheckMenus() {
	EnableAMenuItem(IDM_FILE_SAVE, isDirty);
	EnableAMenuItem(IDM_EDIT_UNDO, SendEditor(EM_CANUNDO));
	EnableAMenuItem(IDM_EDIT_REDO, SendEditor(SCI_CANREDO));
	EnableAMenuItem(IDM_EDIT_PASTE, SendEditor(EM_CANPASTE));
}

void DMApp::Notify(SCNotification *notification) {
	switch (notification->nmhdr.code) {
	case SCN_SAVEPOINTREACHED:
		isDirty = false;
		CheckMenus();
		break;

	case SCN_SAVEPOINTLEFT:
		isDirty = true;
		CheckMenus();
		break;
	}
}

void DMApp::SetAStyle(int style, COLORREF fore, COLORREF back, int size, const char *face) {
	SendEditor(SCI_STYLESETFORE, style, fore);
	SendEditor(SCI_STYLESETBACK, style, back);
	if (size >= 1)
		SendEditor(SCI_STYLESETSIZE, style, size);
	if (face) 
		SendEditor(SCI_STYLESETFONT, style, reinterpret_cast<LPARAM>(face));
}

void DMApp::InitialiseEditor() {
	SendEditor(SCI_SETLEXER, SCLEX_MMIXAL);

	SendEditor(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
	SendEditor(SCI_SETMARGINWIDTHN, 0, (int)SendEditor(SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)"_1") + 10);

	SendEditor(SCI_SETMARGINTYPEN, 1, SC_MARGIN_BACK);
	
	const COLORREF red = RGB(0xFF, 0, 0);
	const COLORREF darkRed = RGB(0x80, 0, 0);
	const COLORREF darkGreen = RGB(0, 0x67, 0);
	const COLORREF lightGreen = RGB(0x30, 0xB0, 0x30);
	const COLORREF green = RGB(0, 0xB0, 0);
	const COLORREF darkLightGreen = RGB(0x30, 0x67, 0x30);
	const COLORREF darkBlue = RGB(0, 0, 0x67);
	const COLORREF blue = RGB(0, 0, 0xA0);
	const COLORREF brown = RGB(0x80, 0x80, 0);
	const COLORREF orange = RGB(0xFF, 0x80, 0);
	const COLORREF lightBlue = RGB(0xA6, 0xCA, 0xF0);
	const COLORREF darkYellow = RGB(0xA0, 0xA0, 0);
	const COLORREF darkGrey = RGB(0x50, 0x50, 0x50);
	const COLORREF lightGrey = RGB(0xA0, 0xA0, 0xA0);
	
	SetAStyle(STYLE_DEFAULT, black, white, 11, "Verdana");
	SendEditor(SCI_STYLECLEARALL);

	SendEditor(SCI_SETCARETLINEVISIBLE, true);
	SendEditor(SCI_SETCARETLINEBACK, nearlyWhite);

	SetAStyle(SCE_MMIXAL_CHAR, brown);
	SetAStyle(SCE_MMIXAL_LABEL, darkGrey);
	SetAStyle(SCE_MMIXAL_OPCODE_PRE, darkLightGreen);
	SetAStyle(SCE_MMIXAL_OPCODE_UNKNOWN, lightGreen);
	SetAStyle(SCE_MMIXAL_OPCODE_VALID, green);
	SetAStyle(SCE_MMIXAL_OPCODE_POST, darkGreen);
	SetAStyle(SCE_MMIXAL_NUMBER, orange);
	SetAStyle(SCE_MMIXAL_SYMBOL, lightBlue);
	SetAStyle(SCE_MMIXAL_STRING, darkYellow);
	SetAStyle(SCE_MMIXAL_COMMENT, lightGrey);
	SetAStyle(SCE_MMIXAL_REF, blue);
	SetAStyle(SCE_MMIXAL_REGISTER, red);
	SetAStyle(SCE_MMIXAL_INCLUDE, darkRed);
	SetAStyle(SCE_MMIXAL_HEX, orange);
	SetAStyle(SCE_MMIXAL_OPERATOR, black);
	SetAStyle(SCE_MMIXAL_OPERANDS, darkBlue);
	SetAStyle(SCE_MMIXAL_LEADWS, black);

	SendEditor(SCI_MARKERDEFINE, ErrorMarker, SC_MARK_FULLRECT);
	SendEditor(SCI_MARKERSETFORE, ErrorMarker, RGB(0x7F, 0x7F, 0x7F));
	SendEditor(SCI_MARKERSETBACK, ErrorMarker, RGB(0xFF, 0, 0));
	SendEditor(SCI_MARKERSETBACKSELECTED, ErrorMarker, RGB(0x7F, 0, 0));
	
	SendEditor(SCI_MARKERDEFINE, BackgroundMarker, SC_MARK_BACKGROUND);
	SendEditor(SCI_MARKERSETBACK, BackgroundMarker, RGB(0x9F, 0x9F, 0x9F));
	SendEditor(SCI_MARKERSETBACKSELECTED, BackgroundMarker, RGB(0x8F, 0x8F, 0x8F));
}

void DMApp::positionCaret(){

	int selection = (int) SendMessage(wErrorList, LB_GETCURSEL, 0, 0);
	int lineNumber = (int) SendMessage(wErrorList, LB_GETITEMDATA, selection, 0);
	SendEditor(SCI_GOTOLINE, lineNumber, 0);
	SetFocus(wEditor);
}

EXTERN_C char* mmixal(FILE* file, char* filename);
EXTERN_C char* mmixsim(FILE* file);
EXTERN_C int interacting;

DWORD WINAPI ConsoleThread(LPVOID lpParam){

	int hConHandle;
	long lStdHandle;

	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE* file;

	AllocConsole();

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = 500;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	file = _fdopen(hConHandle, "w");
	*stdout = *file;
	setvbuf(stdout, NULL, _IONBF, 0);

	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	file = _fdopen(hConHandle, "r");
	*stdin = *file;
	setvbuf(stdin, NULL, _IONBF, 0);

	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	file = _fdopen(hConHandle, "w");
	*stderr = *file;
	setvbuf(stderr, NULL, _IONBF, 0);

	std::ios_base::sync_with_stdio();

	interacting = 1;
	mmixsim((FILE*)lpParam);

	return 0;
}

extern "C" void addListString(char* text, int lineNumber){

	wchar_t* wText = new wchar_t[strlen(text)+1];
	mbstowcs(wText, text, strlen(text)+1);

	int index = (int)SendMessage(app.wErrorList, LB_ADDSTRING, 0, (LPARAM) wText);
	int data = lineNumber - 1;
	SendMessage(app.wErrorList, LB_SETITEMDATA, (WPARAM)index, (LPARAM)data);

	app.SendEditor(SCI_MARKERADD, lineNumber - 1, ErrorMarker);
	app.SendEditor(SCI_MARKERADD, lineNumber - 1, BackgroundMarker);
}

int APIENTRY _tWinMain(HINSTANCE instance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	GetCurrentDirectoryW(256, home);

	app.hInstance = instance;

 	// TODO: Hier Code einfügen.
	MSG msg;
	HACCEL hAccelTable;

	HANDLE handle = LoadLibrary(L"SciLexer.DLL");

	if(!handle){
		LPTSTR lpMsgBuf;
		DWORD dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );

		MessageBox(app.wMain, lpMsgBuf, L"Error", MB_OK);
		return 0;
	}

	// Globale Zeichenfolgen initialisieren
	LoadString(app.hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(app.hInstance, IDC_MMIXEDIT, szWindowClass, MAX_LOADSTRING);
	ATOM atom = MyRegisterClass(app.hInstance);
	if(atom == 0){
		LPTSTR lpMsgBuf;
		DWORD dw = GetLastError(); 

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );

		MessageBox(app.wMain, lpMsgBuf, L"Register Error", MB_OK);
		return 0;
	}
	
	app.wMain = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, app.hInstance, NULL);

	if(!app.wMain){
		LPTSTR lpMsgBuf;
		DWORD dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );

		MessageBox(app.wMain, lpMsgBuf, L"Window Error", MB_OK);
		return 0;
	}

	hAccelTable = LoadAccelerators(app.hInstance, MAKEINTRESOURCE(IDC_MMIXEDIT));

	ShowWindow(app.wMain, nCmdShow);
	UpdateWindow(app.wMain);

	RECT rc;
	::GetClientRect(app.wMain, &rc);

	app.wEditor = CreateWindow(
		            L"Scintilla",
					L"Source",
					WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN | WS_EX_ACCEPTFILES,
		            rc.left, rc.top, 
					rc.right - rc.left, ((rc.bottom - rc.top) / 5) * 4,
					app.wMain,
		            0,
		            app.hInstance,
		            0);

	app.wErrorList = CreateWindow(
					L"ListBox",
					L"Error",
					WS_CHILD | WS_VSCROLL | WS_CLIPCHILDREN | LBS_HASSTRINGS | LBS_NOTIFY,
					rc.left, rc.top + ((rc.bottom - rc.top) / 5) * 4,
					rc.right - rc.left, (rc.bottom - rc.top) / 5,
					app.wMain,
					0,
					app.hInstance,
					0);

	if(!app.wErrorList){
		LPTSTR lpMsgBuf;
		DWORD dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );

		MessageBox(app.wMain, lpMsgBuf, L"List Error", MB_OK);
		return 0;
	}

	app.InitialiseEditor();
	ShowWindow(app.wEditor, SW_SHOWNORMAL);
	ShowWindow(app.wErrorList, SW_SHOWNORMAL);
	SetFocus(app.wEditor);
	UpdateWindow(app.wEditor);
	UpdateWindow(app.wErrorList);

	// Hauptnachrichtenschleife:
	while (GetMessage(&msg, NULL, 0, 0))
	{

		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

void assemble(){
	app.SendEditor(SCI_MARKERDELETEALL, -1);
	SendMessage(app.wErrorList, LB_RESETCONTENT, 0, 0);

	FILE* src_file = _wfopen(app.fullPath, L"rb");
	if(!src_file){
		MessageBox(app.wMain, app.fullPath, L"Can't open mms-file", MB_OK);
		
		LPTSTR lpMsgBuf;
		DWORD dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );

		MessageBox(app.wMain, lpMsgBuf, L"List Error", MB_OK);
	} else {

		char *path = new char[wcslen(app.fullPath) + 1];

		mmixal(src_file, path);
	}
}

void run(){

	wchar_t* filename = app.fullPath;
	filename[wcslen(filename)-1] = 'o';
	FILE* src_file = _wfopen(filename, L"rb");
	if(!src_file){
		MessageBox(app.wMain, app.fullPath, L"Can't open mmo-file", MB_OK);
		
		LPTSTR lpMsgBuf;
		DWORD dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );

		MessageBox(app.wMain, lpMsgBuf, L"MMO Error", MB_OK);
	} else {

		CreateThread(0, 1024, ConsoleThread, (LPVOID) src_file, 0, 0);
	}
}

//
//  FUNKTION: MyRegisterClass()
//
//  ZWECK: Registriert die Fensterklasse.
//
//  KOMMENTARE:
//
//    Sie müssen die Funktion verwenden,  wenn Sie möchten, dass der Code
//    mit Win32-Systemen kompatibel ist, bevor die RegisterClassEx-Funktion
//    zu Windows 95 hinzugefügt wurde. Der Aufruf der Funktion ist wichtig,
//    damit die kleinen Symbole, die mit der Anwendung verknüpft sind,
//    richtig formatiert werden.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MMIXEDIT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MMIXEDIT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//  FUNKTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ZWECK:  Verarbeitet Meldungen vom Hauptfenster.
//
//  WM_COMMAND	- Verarbeiten des Anwendungsmenüs
//  WM_PAINT	- Zeichnen des Hauptfensters
//  WM_DESTROY	- Beenden-Meldung anzeigen und zurückgeben
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, UINT wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	HMENU menuBar, subMenu;

	WORD word;

	LPNMHDR hdr;

	switch (message)
	{
	case WM_CREATE:
		
		return 0;

	case WM_SIZE:
		if (wParam != 1) {
			RECT rc;
			::GetClientRect(hWnd, &rc);
			//SetWindowPos(app.wError, 0, rc.left, rc.top + ((rc.bottom - rc.top) / 5) * 4, rc.right - rc.left, (rc.bottom - rc.top) / 5, 0);
			SetWindowPos(app.wErrorList, 0, rc.left, rc.top + ((rc.bottom - rc.top) / 5) * 4, rc.right - rc.left, (rc.bottom - rc.top) / 5, 0);
			SetWindowPos(app.wEditor, 0, rc.left, rc.top, rc.right - rc.left, ((rc.bottom - rc.top) / 5) * 4, 0);
		}
		return 0;

	case WM_COMMAND:
		app.Command(wParam);
		app.CheckMenus();

		switch(LOWORD(wParam)){

		case 0:
			if(HIWORD(wParam) == LBN_DBLCLK){
				app.positionCaret();
			}
			break;

		case IDD_MENU_COMPILE:
			app.SaveIfUnsure();
			assemble();
			break;

		case IDD_MENU_RUN:
			app.SaveIfUnsure();
			assemble();
			run();
			break;

		case IDM_ABOUT:

			HWND about = CreateDialog(app.hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), app.wMain, About);

			ShowWindow(about, SW_SHOWNORMAL);
		}
		break;

	case WM_NOTIFY:

		app.Notify(reinterpret_cast<SCNotification *>(lParam));
		return 0;

	case WM_MENUSELECT:
		app.CheckMenus();
		return 0;

	case WM_CLOSE:
		if (app.SaveIfUnsure() != IDCANCEL) {
			::DestroyWindow(app.wEditor);
			::PostQuitMessage(0);
		}
		return 0;

	case WM_CTLCOLORLISTBOX:
		if((HWND)lParam == app.wErrorList){

			SetBkColor((HDC)wParam, RGB(0, 0, 0));
			SetTextColor((HDC)wParam, RGB(255, 255, 255));
			return (BOOL)CreateSolidBrush(RGB(0, 0, 0));
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Meldungshandler für Infofeld.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND ErrorHandle;
	HWND ErrorListHandle;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:

		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
