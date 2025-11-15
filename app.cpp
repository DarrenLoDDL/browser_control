#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <cstdlib>
using namespace std;

void SimulateSpaceKey() {
    INPUT input[2] = {};

    // Press Space key down
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = VK_SPACE;

    // Release Space key
    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wVk = VK_SPACE;
    input[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, input, sizeof(INPUT));
}

std::wstring utf8ToUtf16(const std::string& utf8) {
    if (utf8.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &wstr[0], size_needed);
    return wstr;
}

// Copies a UTF-8 string to the Windows clipboard as Unicode text
bool copyToClipboard(const std::string& text) {
    if (!OpenClipboard(nullptr)) return false;
    EmptyClipboard();

    std::wstring wtext = utf8ToUtf16(text);
    size_t size = (wtext.size() + 1) * sizeof(wchar_t);

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!hMem) {
        CloseClipboard();
        return false;
    }

    void* ptr = GlobalLock(hMem);
    memcpy(ptr, wtext.c_str(), size);
    GlobalUnlock(hMem);

    SetClipboardData(CF_UNICODETEXT, hMem);
    CloseClipboard();
    // Do not free hMem after SetClipboardData
    return true;
}

void simulateCtrlV() {
    // Create an array of INPUT structures
    INPUT inputs[4] = {};

    // Press Ctrl down
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;
    inputs[0].ki.dwFlags = 0;

    // Press V down
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'V';
    inputs[1].ki.dwFlags = 0;

    // Release V
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'V';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    // Release Ctrl
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    // Send the input events
    SendInput(4, inputs, sizeof(INPUT));
}

// Simulates a key press and release
void pressKey(BYTE vk, bool bExtended = false) {
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    if (bExtended)
        input.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
    SendInput(1, &input, sizeof(INPUT));
}

void releaseKey(BYTE vk, bool bExtended = false) {
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    if (bExtended)
        input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    SendInput(1, &input, sizeof(INPUT));
}

void TypeString(const std::string& text) {
    for (char ch : text) {
        SHORT vk_and_mod = VkKeyScan(ch);
        if (vk_and_mod == -1) {
            std::cerr << "Cannot map character: " << ch << std::endl;
            continue;
        }
        BYTE vk = vk_and_mod & 0xFF;
        BYTE shiftState = (vk_and_mod >> 8) & 0xFF;

        bool shiftPressed = (shiftState & 1) != 0;
        bool ctrlPressed = (shiftState & 2) != 0;
        bool altPressed = (shiftState & 4) != 0;

        // Press modifiers
        if (ctrlPressed)  pressKey(VK_CONTROL);
        if (altPressed)   pressKey(VK_MENU);
        if (shiftPressed) pressKey(VK_SHIFT);

        // Press key
        pressKey(vk);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        releaseKey(vk);

        // Release modifiers
        if (shiftPressed) releaseKey(VK_SHIFT);
        if (altPressed)   releaseKey(VK_MENU);
        if (ctrlPressed)  releaseKey(VK_CONTROL);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// Presses Enter key
void PressEnter() {
    pressKey(VK_RETURN);                 // press down
    this_thread::sleep_for(std::chrono::milliseconds(200));
    releaseKey(VK_RETURN);               // release key
}

// Opens Chrome
void LaunchChrome() {
    // You can use full path if needed
    system("start chrome");
}

void PressShiftTab(int times) {
    for (int i = 0; i < times; ++i) {
        // Press Shift down
        keybd_event(VK_SHIFT, 0, 0, 0);
        // Press Tab down
        keybd_event(VK_TAB, 0, 0, 0);
        // Release Tab
        keybd_event(VK_TAB, 0, KEYEVENTF_KEYUP, 0);
        // Release Shift
        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
        Sleep(50);  // small delay between presses
    }
}

bool waitForLeftClick() {
    cout << "Waiting for left mouse click...\n";
    while (true) {
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            Sleep(200); // debounce
            return true;
        }
        Sleep(10);
    }
}

// Overwrite the file with new content
bool overwriteFile(const std::string& filepath, const std::string& newContent) {
    std::ofstream file(filepath, std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filepath << std::endl;
        return false;
    }
    file << newContent;
    file.close();
    return true;
}

string getClipboardText() {
    if (!OpenClipboard(nullptr)) return "";

    HANDLE hData = GetClipboardData(CF_TEXT);
    if (!hData) {
        CloseClipboard();
        return "";
    }

    char* text = static_cast<char*>(GlobalLock(hData));
    if (!text) {
        CloseClipboard();
        return "";
    }

    string result(text);
    GlobalUnlock(hData);
    CloseClipboard();
    return result;
}


int main() {

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    string x;
    string y = "Write the code to do this in cpp. Break the steps down if needed. Only include code in your response and nothing else. Please ensure that it works with no edits made on my end.  Pad the code with 30 lines of text that does nothing.";
    cout << "Write a concise command for ChatGPT: "; // Type a number and press enter
    getline(cin, x);
     // Get user input from the keyboard
    std::cout << "Launching Chrome...\n";
    LaunchChrome();

    // Give Chrome time to launch
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "Typing chatgpt.com...\n";

    // Focus address bar with Ctrl+L
    INPUT inputs[4] = {};
    inputs[0].type = inputs[1].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;      // Press down the Ctrl key
    inputs[1].ki.wVk = 'L';             // Press down the 'L' key
    inputs[1].ki.dwFlags = 0;           // 0 means key press (key down)
    inputs[2].type = inputs[3].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'L';             // Release the 'L' key
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[3].ki.wVk = VK_CONTROL;      // Release the Ctrl key
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(4, inputs, sizeof(INPUT));
    this_thread::sleep_for(std::chrono::milliseconds(200));

    // Type the website and press Enter
    TypeString("chatgpt.com");
    PressEnter();
    this_thread::sleep_for(std::chrono::milliseconds(4000));
    //TypeString(combined);
    //PressEnter();
    //this_thread::sleep_for(std::chrono::milliseconds(4000));
    string combined = x + y;
    copyToClipboard(combined);
    simulateCtrlV();
    ////TypeString(combined);
    PressEnter();
    EmptyClipboard();

    //some experiments with where the cursor ends up
    inputs[0].ki.wVk = VK_CONTROL;      // Press down the Ctrl key
    inputs[1].ki.wVk = 'L';             // Press down the 'L' key

    inputs[2].ki.wVk = 'L';             // Release the 'L' key
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[3].ki.wVk = VK_CONTROL;      // Release the Ctrl key
    //find copy clipboard icon and enter
   
    this_thread::sleep_for(std::chrono::milliseconds(1500));
    PressShiftTab(4);
    //this_thread::sleep_for(std::chrono::milliseconds(8000));
    //PressEnter();
    waitForLeftClick();
    //create temporary cpp file.
    this_thread::sleep_for(std::chrono::milliseconds(3000));
    string filename = "temp_program.cpp";

    ofstream ofs(filename);
    if (!ofs) {
        cerr << "Failed to create file." << endl;
        return 1;
    }
    //waitForLeftClick();
    string content = getClipboardText();
    if (!content.empty()) cout << "Content Captured" << endl;
    else cout << "Clipboard is empty or not text.\n";

    string cppFile = "Enter a Location";
    string newCppContent = content;

    if (!overwriteFile(cppFile, newCppContent)) {
        return 1; // exit if failed to write file
    }

    string exeFile = "cursor_temps.exe";
    
    string compileCmd = "g++ \"" + cppFile + "\" -o \"" + exeFile + "\"";
    int compileResult = system(compileCmd.c_str());

    if (compileResult != 0) {
        cerr << "Compilation failed." << endl;
        return 1;
    }

    int runResult = system(exeFile.c_str());
    if (runResult != 0) {
        cerr << "Execution failed." << endl;
        return 1;
    }

    cout << "Done!\n";
    return 0;

}//open google docs and write me a story about dragons, 200 words.
