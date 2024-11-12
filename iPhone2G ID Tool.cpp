#include <windows.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip> 
#include <shlobj.h>

// Global variables
HINSTANCE hInst; // Instance handle
HWND hLabel, hEdit, hButton; // Handles for UI elements

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void displayInfo(HWND hwnd, const std::string& serialNumber, int prodWeek, int prodYear);
int productionWeek(const std::string& serialNumber);
int productionYear(const std::string& serialNumber);
float distgunishBootLoader(int prodWeek, int prodYear);
std::wstring calcMinOS(int prodWeek, int prodYear);
std::wstring prodWeekToMonth(int prodWeek);
void InvalidErrorHandler(HWND hwnd);
void Credits(HWND hwnd);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow) {
    hInst = hInstance; // Store instance handle
    const wchar_t CLASS_NAME[] = L"iPhone2G ID Tool v2.1";

    WNDCLASS wc = { };

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Set initial window size
    int initialWidth = 600;
    int initialHeight = 400;

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"iPhone2G ID Tool v2.1",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZE,
        CW_USEDEFAULT, CW_USEDEFAULT, initialWidth, initialHeight,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    // Disable resizing
    SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~(WS_MAXIMIZEBOX | WS_SIZEBOX));

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

void CenterElements(HWND hwnd) {
    // Get window size
    RECT rect;
    GetClientRect(hwnd, &rect);
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;

    // Define UI element sizes
    int labelWidth = 200;
    int labelHeight = 20;
    int editWidth = 200;
    int editHeight = 25;
    int buttonWidth = 100;
    int buttonHeight = 30;

    // Calculate centered positions
    int labelX = (windowWidth - labelWidth) / 2;
    int labelY = (windowHeight / 2) - (editHeight + buttonHeight + 30); // Adjust Y position to center all elements

    int editX = (windowWidth - editWidth) / 2;
    int editY = labelY + labelHeight + 10;

    int buttonX = (windowWidth - buttonWidth) / 2;
    int buttonY = editY + editHeight + 10;

    // Set positions of UI elements
    SetWindowPos(hLabel, NULL, labelX, labelY, labelWidth, labelHeight, SWP_NOZORDER);
    SetWindowPos(hEdit, NULL, editX, editY, editWidth, editHeight, SWP_NOZORDER);
    SetWindowPos(hButton, NULL, buttonX, buttonY, buttonWidth, buttonHeight, SWP_NOZORDER);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static std::string serialNumber;
    static int prodWeek = 0;
    static int prodYear = 0;

    switch (uMsg) {
    case WM_CREATE: {
        // Create Label
        hLabel = CreateWindow(L"STATIC", L"Enter the serial number:",
            WS_VISIBLE | WS_CHILD,
            0, 0, 0, 0,
            hwnd, NULL, NULL, NULL);

        // Create Edit field
        hEdit = CreateWindow(L"EDIT", NULL,
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            0, 0, 0, 0,
            hwnd, (HMENU)1, NULL, NULL);

        // Create Button
        hButton = CreateWindow(L"BUTTON", L"Submit",
            WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            0, 0, 0, 0,
            hwnd, (HMENU)2, NULL, NULL);

        CenterElements(hwnd);
        break;
    }
    case WM_SIZE: {
        // Handle resizing of the window
        CenterElements(hwnd);
        break;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == 2) {
            wchar_t wSerialNumber[12];
            GetWindowText(GetDlgItem(hwnd, 1), wSerialNumber, 12);

            // Ensure the input length is exactly 11 characters
            if (wcslen(wSerialNumber) != 11) {
                MessageBox(hwnd, L"Error: Serial number must be 11 characters long.", L"Input Error", MB_OK | MB_ICONERROR);
                return 0;
            }

            // Convert wide string to narrow string manually
            serialNumber.clear();
            for (int i = 0; i < 11; i++) {
                serialNumber.push_back(static_cast<char>(wSerialNumber[i]));
            }

            // Ensure the serial number is null-terminated
            serialNumber.push_back('\0');

            prodWeek = productionWeek(serialNumber);
            prodYear = productionYear(serialNumber);

            if (prodWeek == 0 || prodYear == 0) {
                InvalidErrorHandler(hwnd);
            }
            else {
                displayInfo(hwnd, serialNumber, prodWeek, prodYear);
            }
        }
        break;
    }
    case WM_DESTROY: {
        Credits(hwnd);
        PostQuitMessage(0);
        return 0;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// Function to get the directory of the executable
std::wstring GetExecutableDirectory() {
    wchar_t buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    return std::wstring(buffer).substr(0, pos);
}

// Function to display device information in MessageBox and write to devinfo.txt
void displayInfo(HWND hwnd, const std::string& serialNumber, int prodWeek, int prodYear) {
    std::wstring info;
    info += L"iPhone 2G Information:\n\n";

    if (prodWeek == 0 || prodYear == 0) {
        info += L"Error: Invalid or unknown Serial number.\n";
    }
    else {
        info += L"Serial Number: ";
        info += std::wstring(serialNumber.begin(), serialNumber.end()) + L"\n";
        info += L"Production Week: " + std::to_wstring(prodWeek) + L"\n";
        info += L"Production Month / Year: " + prodWeekToMonth(prodWeek) + L" " + std::to_wstring(prodYear) + L"\n";

        // Get the bootloader version and format it
        float bootloaderVersion = distgunishBootLoader(prodWeek, prodYear);
        std::wstringstream ss;
        ss << std::fixed << std::setprecision(3) << bootloaderVersion;
        std::wstring formattedBootloader = ss.str();

        info += L"Original Bootloader Version: " + formattedBootloader + L"\n";
        info += L"Minimum OS Version: " + calcMinOS(prodWeek, prodYear) + L"\n";
    }

    // Display information in a MessageBox
    MessageBox(hwnd, info.c_str(), L"iPhone 2G Information", MB_OK);

    // Get executable directory
    std::wstring execDir = GetExecutableDirectory();

    // Construct path for devinfo.txt next to the executable
    std::wstring filePath = execDir + L"\\iPhone2G ID Tool.txt";

    // Write information to devinfo.txt file using Windows API
    HANDLE hFile = CreateFile(filePath.c_str(),            // File path
        GENERIC_WRITE,               // Open for writing
        0,                           // Do not share
        NULL,                        // Default security
        CREATE_ALWAYS,               // Always create new file
        FILE_ATTRIBUTE_NORMAL,       // Normal file
        NULL);                       // No template file

    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBox(NULL, L"Failed to create iPhone2GInfo.txt!", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    std::wstring outputText = info;

    // Write information to devinfo.txt file
    DWORD bytesWritten;
    BOOL result = WriteFile(hFile, outputText.c_str(), outputText.length() * sizeof(wchar_t), &bytesWritten, NULL);
    CloseHandle(hFile);

    if (!result) {
        MessageBox(NULL, L"Failed to write to iPhone2GInfo.txt!", L"Error", MB_OK | MB_ICONERROR);
    }
}

// Error handler function
void InvalidErrorHandler(HWND hwnd) {
    MessageBox(hwnd, L"Error: The serial number entered is invalid. Please enter a valid serial number (e.g., 7S738TA00T2).", L"Error", MB_OK | MB_ICONERROR);
}

int productionWeek(const std::string& serialNumber) {
    int week = ((static_cast<int>(serialNumber[3]) - '0') * 10) + (static_cast<int>(serialNumber[4]) - '0');
    if (week > 52 || week <= 0) {
        return 0;
    }
    return week;
}

int productionYear(const std::string& serialNumber) {
    switch (static_cast<int>(serialNumber[2]) - '0') {
    case 7: return 2007;
    case 8: return 2008;
    default: return 0;
    }
}

float distgunishBootLoader(int prodWeek, int prodYear) {
    if ((prodWeek >= 45 && prodYear == 2007) || (prodYear == 2008)) {
        return 4.6f;
    }
    return 3.9f;
}

std::wstring calcMinOS(int prodWeek, int prodYear) {
    if (prodWeek <= 48 && prodYear == 2007) {
        return L"Firmware 1.0";
    }
    return L"Firmware 1.1.1";
}

std::wstring prodWeekToMonth(int prodWeek) {
    if (prodWeek <= 5) return L"January";
    if (prodWeek <= 9) return L"February";
    if (prodWeek <= 13) return L"March";
    if (prodWeek <= 18) return L"April";
    if (prodWeek <= 22) return L"May";
    if (prodWeek <= 26) return L"June";
    if (prodWeek <= 31) return L"July";
    if (prodWeek <= 35) return L"August";
    if (prodWeek <= 39) return L"September";
    if (prodWeek <= 44) return L"October";
    if (prodWeek <= 48) return L"November";
    if (prodWeek <= 52) return L"December";

    return L"Unknown";
}

void Credits(HWND hwnd) {
    MessageBox(hwnd, L"Credits:\n randomNinja64 - Initial code Creator of the code\n Wubbl0rzDev - Elaboration Basic Programming\n BJNFNE - Elaboration of the code\n JustGhost101 - Icon design\n", L"iPhone2G ID Tool v2.1", MB_ICONASTERISK);
    return;
}
