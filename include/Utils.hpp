#pragma once

#include <string>
#include<vector>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>


std::string openFileDialog(HWND owner = NULL,bool img=true);
std::string saveFileDialog(HWND owner, const char* filter, const char* defaultFileName);
#endif
bool writeBufferToFile(const std::string& filePath, const std::vector<char>& buffer);