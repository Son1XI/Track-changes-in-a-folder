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
	start = chrono::system_clock::now(); // Запоминаем текущее время
	time_t start_time = chrono::system_clock::to_time_t(start);
	cout << "Started at: " << ctime(&start_time) << endl;
	end = start;
	end += chrono::seconds(10); // Время до которого программа будет собирать информацию
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
		
		if (result) // Получили сообщение об изменении в отслеживаемой директории
		{
			memset(file, 0, strlen(file)); // Убираем мусор что бы не возникали лишние символы (Что будет без этого: Новая папкаМММММММММ)
			WideCharToMultiByte(CP_ACP, 0, pNotify->FileName, pNotify->FileNameLength / 2, file, 260, NULL, NULL); // Переводит wchar от FILE_NOTIFY_INFORMATION в char
			if (pNotify->Action == FILE_ACTION_ADDED) { // Если был добавлен файл или папка
				string addChange = "File Added: ";
				allChanges.push_back(addChange);
			}
			else if (pNotify->Action == FILE_ACTION_REMOVED) { // Если удален файл или папка
				string remChange = "File Deleted: ";
				allChanges.push_back(remChange);
			}
			else if (pNotify->Action == FILE_ACTION_MODIFIED) { // Если изменили файл (записали/удалили содержимое)
				string modChange = "File Modified: ";
				allChanges.push_back(modChange);
			}
			else if (pNotify->Action == FILE_ACTION_RENAMED_OLD_NAME) { // Если переименовали файл или папку
				string renChange = "File Renamed: ";
				allChanges.push_back(renChange);
			}
			allChanges.push_back(file);
		}
		now = chrono::system_clock::now();
	}
	cout << "Time have passed! What changes:" << endl;
	int i = 1;
	for (const auto& str : allChanges) // Вывод всех изменений за 60 секунд
	{
		cout << str;
		if (i % 2 == 0)
			cout << "\n";
		i++;
	}
}


int main() {
	setlocale(LC_ALL, ""); // Для корректного отображения символов в консоли
	string dir = getDirectory() + "\\check"; // Получаем директорию папки check
	cout << "Will check the changes in: " << dir << endl;
	checkForChanges(dir); // Запускаем отслеживание изменений в полученной директории
	return 0;
}