// MMIX-Edit.cpp : Definiert den Einstiegspunkt für die Anwendung.
//

#include "stdafx.h"
#include "MMIX-Edit.h"
#include "SciLexer.h"
#include "Scintilla.h"
#include <Windows.h>
#include <stdio.h>

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
	void positionCursor();

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
	SendMessage(wEditor, EM_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));
}

void DMApp::SetTitle() {
	wchar_t title[MAX_PATH + 100];
	swprintf(title, appName, NULL);
	wcscat(title, L" - ");
	wcscat(title, fullPath);
	::SetWindowText(wMain, title);
}

void DMApp::New() {
	SendEditor(SCI_CLEARALL);
	SendEditor(EM_EMPTYUNDOBUFFER);
	fullPath[0] = '\0';
	SetTitle();
	isDirty = false;
	SendEditor(SCI_SETSAVEPOINT);
}

void DMApp::OpenFile(const wchar_t *fileName) {

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

	case 65536:
		if(HIWORD(id) == LBN_DBLCLK)
			positionCursor();
		break;

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

const char htmlKeyWords[] = 
	"a abbr acronym address applet area b base basefont "
	"bdo big blockquote body br button caption center "
	"cite code col colgroup dd del dfn dir div dl dt em "
	"fieldset font form frame frameset h1 h2 h3 h4 h5 h6 "
	"head hr html i iframe img input ins isindex kbd label "
	"legend li link map menu meta noframes noscript "
	"object ol optgroup option p param pre q s samp "
	"script select small span strike strong style sub sup "
	"table tbody td textarea tfoot th thead title tr tt u ul "
	"var xmlns "
	"abbr accept-charset accept accesskey action align alink "
	"alt archive axis background bgcolor border "
	"cellpadding cellspacing char charoff charset checked cite "
	"class classid clear codebase codetype color cols colspan "
	"compact content coords "
	"data datafld dataformatas datapagesize datasrc datetime "
	"declare defer dir disabled enctype "
	"face for frame frameborder "
	"headers height href hreflang hspace http-equiv "
	"id ismap label lang language link longdesc "
	"marginwidth marginheight maxlength media method multiple "
	"name nohref noresize noshade nowrap "
	"object onblur onchange onclick ondblclick onfocus "
	"onkeydown onkeypress onkeyup onload onmousedown "
	"onmousemove onmouseover onmouseout onmouseup "
	"onreset onselect onsubmit onunload "
	"profile prompt readonly rel rev rows rowspan rules "
	"scheme scope shape size span src standby start style "
	"summary tabindex target text title type usemap "
	"valign value valuetype version vlink vspace width "
	"text password checkbox radio submit reset "
	"file hidden image "
	"public !doctype xml";

const char jsKeyWords[] = 
	"break case catch continue default "
	"do else for function if return throw try var while";

const char vbsKeyWords[] = 
	"and as byref byval case call const "
	"continue dim do each else elseif end error exit false for function global "
	"goto if in loop me new next not nothing on optional or private public "
	"redim rem resume select set sub then to true type while with "
	"boolean byte currency date double integer long object single string type "
	"variant";

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
	SendEditor(SCI_SETSTYLEBITS, 7);

	SendEditor(SCI_SETKEYWORDS, 0, 
		reinterpret_cast<LPARAM>(htmlKeyWords));
	SendEditor(SCI_SETKEYWORDS, 1, 
		reinterpret_cast<LPARAM>(jsKeyWords));
	SendEditor(SCI_SETKEYWORDS, 2, 
		reinterpret_cast<LPARAM>(vbsKeyWords));

	// Set up the global default style. These attributes are used wherever no explicit choices are made.
	SetAStyle(STYLE_DEFAULT, black, white, 11, "Verdana");
	SendEditor(SCI_STYLECLEARALL);	// Copies global style to all others

	const COLORREF red = RGB(0xFF, 0, 0);
	const COLORREF offWhite = RGB(0xFF, 0xFB, 0xF0);
	const COLORREF darkGreen = RGB(0, 0x80, 0);
	const COLORREF darkBlue = RGB(0, 0, 0x80);

	// Hypertext default is used for all the document's text
	SetAStyle(SCE_H_DEFAULT, black, white, 11, "Times New Roman");
	
	// Unknown tags and attributes are highlighed in red. 
	// If a tag is actually OK, it should be added in lower case to the htmlKeyWords string.
	SetAStyle(SCE_H_TAG, darkBlue);
	SetAStyle(SCE_H_TAGUNKNOWN, red);
	SetAStyle(SCE_H_ATTRIBUTE, darkBlue);
	SetAStyle(SCE_H_ATTRIBUTEUNKNOWN, red);
	SetAStyle(SCE_H_NUMBER, RGB(0x80,0,0x80));
	SetAStyle(SCE_H_DOUBLESTRING, RGB(0,0x80,0));
	SetAStyle(SCE_H_SINGLESTRING, RGB(0,0x80,0));
	SetAStyle(SCE_H_OTHER, RGB(0x80,0,0x80));
	SetAStyle(SCE_H_COMMENT, RGB(0x80,0x80,0));
	SetAStyle(SCE_H_ENTITY, RGB(0x80,0,0x80));

	SetAStyle(SCE_H_TAGEND, darkBlue);
	SetAStyle(SCE_H_XMLSTART, darkBlue);	// <?
	SetAStyle(SCE_H_XMLEND, darkBlue);		// ?>
	SetAStyle(SCE_H_SCRIPT, darkBlue);		// <script
	SetAStyle(SCE_H_ASP, RGB(0x4F, 0x4F, 0), RGB(0xFF, 0xFF, 0));	// <% ... %>
	SetAStyle(SCE_H_ASPAT, RGB(0x4F, 0x4F, 0), RGB(0xFF, 0xFF, 0));	// <%@ ... %>

	SetAStyle(SCE_HB_DEFAULT, black);
	SetAStyle(SCE_HB_COMMENTLINE, darkGreen);
	SetAStyle(SCE_HB_NUMBER, RGB(0,0x80,0x80));
	SetAStyle(SCE_HB_WORD, darkBlue);
	SendEditor(SCI_STYLESETBOLD, SCE_HB_WORD, 1);
	SetAStyle(SCE_HB_STRING, RGB(0x80,0,0x80));
	SetAStyle(SCE_HB_IDENTIFIER, black);
	
	// This light blue is found in the windows system palette so is safe to use even in 256 colour modes.
	const COLORREF lightBlue = RGB(0xA6, 0xCA, 0xF0);
	// Show the whole section of VBScript with light blue background
	for (int bstyle=SCE_HB_DEFAULT; bstyle<=SCE_HB_STRINGEOL; bstyle++) {
		SendEditor(SCI_STYLESETFONT, bstyle, 
			reinterpret_cast<LPARAM>("Georgia"));
		SendEditor(SCI_STYLESETBACK, bstyle, lightBlue);
		// This call extends the backround colour of the last style on the line to the edge of the window
		SendEditor(SCI_STYLESETEOLFILLED, bstyle, 1);
	}
	SendEditor(SCI_STYLESETBACK, SCE_HB_STRINGEOL, RGB(0x7F,0x7F,0xFF));
	SendEditor(SCI_STYLESETFONT, SCE_HB_COMMENTLINE, 
		reinterpret_cast<LPARAM>("Comic Sans MS"));

	SetAStyle(SCE_HBA_DEFAULT, black);
	SetAStyle(SCE_HBA_COMMENTLINE, darkGreen);
	SetAStyle(SCE_HBA_NUMBER, RGB(0,0x80,0x80));
	SetAStyle(SCE_HBA_WORD, darkBlue);
	SendEditor(SCI_STYLESETBOLD, SCE_HBA_WORD, 1);
	SetAStyle(SCE_HBA_STRING, RGB(0x80,0,0x80));
	SetAStyle(SCE_HBA_IDENTIFIER, black);
	
	// Show the whole section of ASP VBScript with bright yellow background
	for (int bastyle=SCE_HBA_DEFAULT; bastyle<=SCE_HBA_STRINGEOL; bastyle++) {
		SendEditor(SCI_STYLESETFONT, bastyle, 
			reinterpret_cast<LPARAM>("Georgia"));
		SendEditor(SCI_STYLESETBACK, bastyle, RGB(0xFF, 0xFF, 0));
		// This call extends the backround colour of the last style on the line to the edge of the window
		SendEditor(SCI_STYLESETEOLFILLED, bastyle, 1);
	}
	SendEditor(SCI_STYLESETBACK, SCE_HBA_STRINGEOL, RGB(0xCF,0xCF,0x7F));
	SendEditor(SCI_STYLESETFONT, SCE_HBA_COMMENTLINE, 
		reinterpret_cast<LPARAM>("Comic Sans MS"));
		
	// If there is no need to support embedded Javascript, the following code can be dropped.
	// Javascript will still be correctly processed but will be displayed in just the default style.
	
	SetAStyle(SCE_HJ_START, RGB(0x80,0x80,0));
	SetAStyle(SCE_HJ_DEFAULT, black);
	SetAStyle(SCE_HJ_COMMENT, darkGreen);
	SetAStyle(SCE_HJ_COMMENTLINE, darkGreen);
	SetAStyle(SCE_HJ_COMMENTDOC, darkGreen);
	SetAStyle(SCE_HJ_NUMBER, RGB(0,0x80,0x80));
	SetAStyle(SCE_HJ_WORD, black);
	SetAStyle(SCE_HJ_KEYWORD, darkBlue);
	SetAStyle(SCE_HJ_DOUBLESTRING, RGB(0x80,0,0x80));
	SetAStyle(SCE_HJ_SINGLESTRING, RGB(0x80,0,0x80));
	SetAStyle(SCE_HJ_SYMBOLS, black);

	SetAStyle(SCE_HJA_START, RGB(0x80,0x80,0));
	SetAStyle(SCE_HJA_DEFAULT, black);
	SetAStyle(SCE_HJA_COMMENT, darkGreen);
	SetAStyle(SCE_HJA_COMMENTLINE, darkGreen);
	SetAStyle(SCE_HJA_COMMENTDOC, darkGreen);
	SetAStyle(SCE_HJA_NUMBER, RGB(0,0x80,0x80));
	SetAStyle(SCE_HJA_WORD, black);
	SetAStyle(SCE_HJA_KEYWORD, darkBlue);
	SetAStyle(SCE_HJA_DOUBLESTRING, RGB(0x80,0,0x80));
	SetAStyle(SCE_HJA_SINGLESTRING, RGB(0x80,0,0x80));
	SetAStyle(SCE_HJA_SYMBOLS, black);

	// Show the whole section of Javascript with off white background
	for (int jstyle=SCE_HJ_DEFAULT; jstyle<=SCE_HJ_SYMBOLS; jstyle++) {
		SendEditor(SCI_STYLESETFONT, jstyle, 
			reinterpret_cast<LPARAM>("Lucida Sans Unicode"));
		SendEditor(SCI_STYLESETBACK, jstyle, offWhite);
		SendEditor(SCI_STYLESETEOLFILLED, jstyle, 1);
	}
	SendEditor(SCI_STYLESETBACK, SCE_HJ_STRINGEOL, RGB(0xDF, 0xDF, 0x7F));
	SendEditor(SCI_STYLESETEOLFILLED, SCE_HJ_STRINGEOL, 1);

	// Show the whole section of Javascript with brown background
	for (int jastyle=SCE_HJA_DEFAULT; jastyle<=SCE_HJA_SYMBOLS; jastyle++) {
		SendEditor(SCI_STYLESETFONT, jastyle, 
			reinterpret_cast<LPARAM>("Lucida Sans Unicode"));
		SendEditor(SCI_STYLESETBACK, jastyle, RGB(0xDF, 0xDF, 0x7F));
		SendEditor(SCI_STYLESETEOLFILLED, jastyle, 1);
	}
	SendEditor(SCI_STYLESETBACK, SCE_HJA_STRINGEOL, RGB(0x0,0xAF,0x5F));
	SendEditor(SCI_STYLESETEOLFILLED, SCE_HJA_STRINGEOL, 1);

	SendEditor(SCI_MARKERDEFINE, ErrorMarker, SC_MARK_CIRCLE);
	SendEditor(SCI_MARKERSETFORE, ErrorMarker, 0|0|0);
	SendEditor(SCI_MARKERSETBACK, ErrorMarker, 255|0|0);
	SendEditor(SCI_MARKERSETBACKSELECTED, ErrorMarker, 128|0|0);

	SendEditor(SCI_MARKERDEFINE, BackgroundMarker, SC_MARK_BACKGROUND);
	SendEditor(SCI_MARKERSETFORE, BackgroundMarker, 0|0|0);
	SendEditor(SCI_MARKERSETBACK, BackgroundMarker, 128|0|0);
	SendEditor(SCI_MARKERSETBACKSELECTED, BackgroundMarker, 128|0|0);
}

void DMApp::positionCursor(){
	// ERR: SendMessage
	int selection = (int) SendMessage(wErrorList, LB_GETCURSEL, 0, 0);
	int lineNumber = (int) SendMessage(wErrorList, LB_GETITEMDATA, selection, 0);
	SetFocus(wEditor);
	SendEditor(SCI_GOTOLINE, 0, lineNumber);
}

EXTERN_C char* mmixal(FILE* file, char* filename);
EXTERN_C char* mmixsim(FILE* file);

DWORD WINAPI ConsoleThread(LPVOID lpParam){

	if(AllocConsole()){

		printf("look at this:");
		mmixsim((FILE*)lpParam);

	}else{

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

		MessageBox(app.wMain, lpMsgBuf, L"Could not start console", MB_OK);
	}

	return 0;
}

extern "C" void addListString(char* text, int lineNumber, bool isError){

	wchar_t* wText = new wchar_t[strlen(text)+1];
	mbstowcs(wText, text, strlen(text)+1);

	SendMessage(app.wErrorList, LB_ADDSTRING, 0, (LPARAM) wText);
	int index = (int)SendMessage(app.wErrorList, LB_FINDSTRING, -1, (LPARAM)wText);
	SendMessage(app.wErrorList, LB_SETITEMDATA, index, lineNumber - 1);

	app.SendEditor(SCI_MARKERADD, lineNumber - 1, ErrorMarker);
	app.SendEditor(SCI_MARKERADD, lineNumber - 1, BackgroundMarker);
}

INT_PTR CALLBACK DialogProc( HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam ){
    switch( iMessage ){
    case WM_COMMAND:
        switch( LOWORD( wParam ) ){
        }
        break;
    }
    return FALSE;
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

	//app.wError = CreateDialog(app.hInstance, MAKEINTRESOURCE(IDD_ERROR), app.wMain, NULL);
	//app.wErrorList = GetDlgItem(app.wError, IDC_ERRORLIST);

	app.wErrorList = CreateWindow(
					L"ListBox",
					NULL,
					WS_CHILD | WS_VSCROLL | WS_CLIPCHILDREN | LBS_HASSTRINGS | LBS_NOTIFY,
					rc.left, rc.top + ((rc.bottom - rc.top) / 5) * 4,
					rc.right - rc.left, (rc.bottom - rc.top) / 5,
					app.wMain,
					0,
					app.hInstance,
					0);
	
	//SetParent(app.wErrorList, app.wMain);

	//SetWindowPos(app.wErrorList, 0, rc.left, rc.top + ((rc.bottom - rc.top) / 5) * 4, rc.right - rc.left, (rc.bottom - rc.top) / 5, 0);

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

		wcstombs(path, app.fullPath, wcslen(app.fullPath) + 1);
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
				app.positionCursor();
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

		case IDC_ERRORLIST:
			if(HIWORD(wParam) == LBN_DBLCLK){
				
				app.positionCursor();
			}
			break;

		case IDM_ABOUT:

			HWND about = CreateDialog(app.hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), app.wMain, NULL);

			ShowWindow(about, SW_SHOWNORMAL);
		}
		break;

	case WM_NOTIFY:

		hdr = (LPNMHDR)lParam;

		switch(hdr->code){
			
			case LBN_SETFOCUS:
				app.positionCursor();
				break;

			case LBN_DBLCLK:
				app.positionCursor();
				break;

			default:
				break;
		}

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
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Hier den Zeichnungscode hinzufügen.
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

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

		/*ErrorHandle = GetDlgItem(app.wMain, IDD_ERROR);
		ErrorListHandle = GetDlgItem(app.wError, IDC_ERRORLIST);

		RECT rc;
		::GetClientRect(hDlg, &rc);
		SetWindowPos(ErrorListHandle, 0, rc.left, rc.top + ((rc.bottom - rc.top) / 5) * 4, rc.right - rc.left, (rc.bottom - rc.top) / 5, 0);
		SetWindowPos(ErrorHandle, 0, rc.left, rc.top + ((rc.bottom - rc.top) / 5) * 4, rc.right - rc.left, (rc.bottom - rc.top) / 5, 0);
		*/
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
