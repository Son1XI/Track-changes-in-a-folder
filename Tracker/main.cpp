#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <string>
#include <ctime>
#include <chrono>
#include <vector>
#include <thread>

using namespace std;


string getDirectory() {
	TCHAR buffer[MAX_PATH];
	GetCurrentDirectory(sizeof(buffer), buffer);
	string projDir = buffer;
	return projDir;
}

void checkForChanges(string dir) {
	DWORD returnedB = 0;
	BOOL result;
	char buf[1024];
	char file[MAX_PATH] = "";
	chrono::time_point<chrono::system_clock> start, end, now;
	vector <string> allChanges;

	HANDLE hDir = CreateFile(dir.c_str(),
		GENERIC_READ | GENERIC_WRITE | FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL);

	if (hDir == INVALID_HANDLE_VALUE)
	{
		cout << GetLastError() << endl;
		return;
	}

	FILE_NOTIFY_INFORMATION *pNotify = (FILE_NOTIFY_INFORMATION*)buf;

	cout << "Successfully launched! Wait 60 sec." << endl;
	start = chrono::system_clock::now(); // ���������� ������� �����
	time_t start_time = chrono::system_clock::to_time_t(start);
	cout << "Started at: " << ctime(&start_time) << endl;
	end = start;
	end += chrono::seconds(10); // ����� �� �������� ��������� ����� �������� ����������
	time_t end_time = chrono::system_clock::to_time_t(end);
	cout << "Will end at: " << ctime(&end_time) << endl;

	
	while (now < end) {
		result = ReadDirectoryChangesW(hDir,
			&buf,
			sizeof(buf),
			TRUE,
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_DIR_NAME,
			&returnedB,
			NULL,
			NULL);
		
		if (result) // �������� ��������� �� ��������� � ������������� ����������
		{
			memset(file, 0, strlen(file)); // ������� ����� ��� �� �� ��������� ������ ������� (��� ����� ��� �����: ����� ��������������)
			WideCharToMultiByte(CP_ACP, 0, pNotify->FileName, pNotify->FileNameLength / 2, file, 260, NULL, NULL); // ��������� wchar �� FILE_NOTIFY_INFORMATION � char
			if (pNotify->Action == FILE_ACTION_ADDED) { // ���� ��� �������� ���� ��� �����
				string addChange = "File Added: ";
				allChanges.push_back(addChange);
			}
			else if (pNotify->Action == FILE_ACTION_REMOVED) { // ���� ������ ���� ��� �����
				string remChange = "File Deleted: ";
				allChanges.push_back(remChange);
			}
			else if (pNotify->Action == FILE_ACTION_MODIFIED) { // ���� �������� ���� (��������/������� ����������)
				string modChange = "File Modified: ";
				allChanges.push_back(modChange);
			}
			else if (pNotify->Action == FILE_ACTION_RENAMED_OLD_NAME) { // ���� ������������� ���� ��� �����
				string renChange = "File Renamed: ";
				allChanges.push_back(renChange);
			}
			allChanges.push_back(file);
		}
		now = chrono::system_clock::now();
	}
	cout << "Time have passed! What changes:" << endl;
	int i = 1;
	for (const auto& str : allChanges) // ����� ���� ��������� �� 60 ������
	{
		cout << str;
		if (i % 2 == 0)
			cout << "\n";
		i++;
	}
}


int main() {
	setlocale(LC_ALL, ""); // ��� ����������� ����������� �������� � �������
	string dir = getDirectory() + "\\check"; // �������� ���������� ����� check
	cout << "Will check the changes in: " << dir << endl;
	checkForChanges(dir); // ��������� ������������ ��������� � ���������� ����������
	return 0;
}