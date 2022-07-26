#pragma once

int GetUserQuery(CString filename);
double getElapsedTime(unsigned long long start, int unit);
unsigned long long getCurrTime(char* timeref);
void getCurrDateTimeString(wchar_t* datetime);
int browseForFolder(CString rootdir, TCHAR* szPath);
