#include "../include/Utils.hpp"
#include <iostream>
#include <vector>
#include <fstream>

#ifdef _WIN32
std::string openFileDialog(HWND owner, bool img)
{
    OPENFILENAMEA ofn;
    CHAR szFile[MAX_PATH] = { 0 };
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = owner;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = img ? "Image Files\0*.png;*.jpg;*.jpeg;*.bmp\0All Files\0*.*\0\0" : "All Files\0*.*\0\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetOpenFileNameA(&ofn) == TRUE) {
        return ofn.lpstrFile;
    }
    return std::string();
}

std::string saveFileDialog(HWND owner, const char* filter, const char* defaultFileName) {
    OPENFILENAMEA ofn;
    CHAR szFile[MAX_PATH] = { 0 };

    if (defaultFileName) {
        strncpy_s(szFile, MAX_PATH, defaultFileName, _TRUNCATE);
    }

    ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = owner;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    if (GetSaveFileNameA(&ofn) == TRUE) {
        return ofn.lpstrFile;
    }

    return "";
}
#endif

bool writeBufferToFile(const std::string& filePath, const std::vector<char>& buffer) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Could not open file for writing: " << filePath << std::endl;
        return false;
    }
    file.write(buffer.data(), buffer.size());
    if (!file) {
        std::cerr << "Error writing to file: " << filePath << std::endl;
        return false;
    }
    return true;
}