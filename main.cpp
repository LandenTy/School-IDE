#include <windows.h>
#include <commctrl.h>
#include <fstream>
#include <string>
#include <filesystem>

// Global variables
HWND g_hEdit; // Edit control handle
OPENFILENAME g_ofn; // Open and Save file dialog structure

#define IDC_MAIN_EDIT 1001 // Define IDC_MAIN_EDIT constant
#define ID_FILE_OPEN  9001 // Define ID_FILE_OPEN constant
#define ID_FILE_SAVE  9002 // Define ID_FILE_SAVE constant
#define ID_EDIT_SELECTALL  9003

// Function prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void DoFileOpen(HWND hwnd);
void DoFileSave(HWND hwnd);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
    static TCHAR szAppName[] = TEXT("SimpleEditor");
    HWND hwnd;
    MSG msg;
    WNDCLASS wndclass;

    // Define window class
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;

    // Register window class
    if (!RegisterClass(&wndclass))
    {
        MessageBox(NULL, TEXT("This program requires Windows NT!"), szAppName, MB_ICONERROR);
        return 0;
    }

    // Create window
    hwnd = CreateWindow(szAppName, TEXT("Simple IDE"), WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    // Message loop
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        // Create edit control
        g_hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""),
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
            ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            0, 0, 0, 0,
            hwnd, (HMENU)IDC_MAIN_EDIT,
            ((LPCREATESTRUCT)lParam)->hInstance, NULL);

        // Create menu
        HMENU hMenu, hSubMenu;
        hMenu = CreateMenu();
        hSubMenu = CreatePopupMenu();
        AppendMenu(hSubMenu, MF_STRING, ID_FILE_OPEN, "Open");
        AppendMenu(hSubMenu, MF_STRING, ID_FILE_SAVE, "Save");
        AppendMenu(hMenu, MF_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(hSubMenu), "File");

        // Add Select All option to Edit menu
        hSubMenu = CreatePopupMenu();
        AppendMenu(hSubMenu, MF_STRING, ID_EDIT_SELECTALL, "Select All");
        AppendMenu(hMenu, MF_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(hSubMenu), "Edit");

        SetMenu(hwnd, hMenu);

        break;

    case WM_SIZE:
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);
        MoveWindow(g_hEdit, 0, 0, rcClient.right, rcClient.bottom, TRUE);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_FILE_OPEN:
            DoFileOpen(hwnd);
            break;
        case ID_FILE_SAVE:
            DoFileSave(hwnd);
            break;
        case ID_EDIT_SELECTALL:
            SendMessage(g_hEdit, EM_SETSEL, 0, -1);
            break;
        }
        break;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_KEYDOWN:
        if (wParam == 'A' && GetKeyState(VK_CONTROL) & 0x8000) {
            // CTRL+A is pressed
            // Select all text in the edit control
            SendMessage(g_hEdit, EM_SETSEL, 0, -1);
        }
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

void DoFileOpen(HWND hwnd)
{
    TCHAR szFileName[MAX_PATH] = TEXT("");

    // Initialize OPENFILENAME
    ZeroMemory(&g_ofn, sizeof(g_ofn));
    g_ofn.lStructSize = sizeof(g_ofn);
    g_ofn.hwndOwner = hwnd;
    g_ofn.lpstrFilter = TEXT("Text Files (*.txt)\0*.TXT\0All Files (*.*)\0*.*\0");
    g_ofn.lpstrFile = szFileName;
    g_ofn.nMaxFile = MAX_PATH;
    g_ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    g_ofn.lpstrDefExt = TEXT("txt");

    // Display the Open dialog box
    if (GetOpenFileName(&g_ofn) == TRUE)
    {
        // Open the file and read its content
        std::ifstream file(g_ofn.lpstrFile);
        if (file.is_open())
        {
            std::string content((std::istreambuf_iterator<char>(file)),
                                (std::istreambuf_iterator<char>()));
            SetWindowText(g_hEdit, content.c_str());
            file.close();
        }
    }
}

void DoFileSave(HWND hwnd)
{
    TCHAR szFileName[MAX_PATH] = TEXT("");

    // Initialize OPENFILENAME
    ZeroMemory(&g_ofn, sizeof(g_ofn));
    g_ofn.lStructSize = sizeof(g_ofn);
    g_ofn.hwndOwner = hwnd;
    g_ofn.lpstrFilter = TEXT("Text Files (*.txt)\0*.TXT\0All Files (*.*)\0*.*\0");
    g_ofn.lpstrFile = szFileName;
    g_ofn.nMaxFile = MAX_PATH;
    g_ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    g_ofn.lpstrDefExt = TEXT("txt");

    // Display the Save As dialog box
    if (GetSaveFileName(&g_ofn) == TRUE)
    {
        // Save the content of the edit control to the file
        std::ofstream file(g_ofn.lpstrFile);
        if (file.is_open())
        {
            int length = GetWindowTextLength(g_hEdit);
            std::string buffer(length + 1, '\0');
            GetWindowText(g_hEdit, &buffer[0], length + 1);
            file << buffer;
            file.close();
        }
    }
}
