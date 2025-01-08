#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>

inline bool file_exists(const std::string &path) {
  struct stat buffer;
  return(stat(path.c_str(), &buffer) == 0);
}

LPCSTR get_full_path(LPCSTR relative_path) {
    static char full_path[MAX_PATH];
    GetFullPathName(relative_path, MAX_PATH, full_path, NULL);
    return full_path;
}

void list_processids() {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(decltype(entry));

    const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    while (Process32Next(snapshot, &entry)) {
        std::cout << "[P] " << entry.szExeFile << " ID: " << entry.th32ProcessID << std::endl;
    }

    if (snapshot) { CloseHandle(snapshot); }
}

std::uintptr_t get_process_id(std::string process_name) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(decltype(entry));

    const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    while (Process32Next(snapshot, &entry)) {
        if (!process_name.compare(entry.szExeFile)) {
            return entry.th32ProcessID;
            break;
        }
    }

    if (snapshot) { CloseHandle(snapshot); }
}

bool inject(LPCSTR dll_path, std::uintptr_t process_id) {
    bool status = false;

    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, 0, process_id);

    PVOID allocated = VirtualAllocEx(handle, nullptr, sizeof(dll_path), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (allocated == 0) { status = false; CloseHandle(handle); return status; }

    status = WriteProcessMemory(handle, allocated, dll_path, sizeof(dll_path), NULL);

    HANDLE thread = CreateRemoteThread(handle, nullptr, NULL, LPTHREAD_START_ROUTINE(LoadLibraryA), allocated, NULL, nullptr);
    if (thread == 0) { status = false; CloseHandle(handle); return status; }

    CloseHandle(thread);
    CloseHandle(handle);

    return status;
}

void command_handler(std::unordered_map<std::string, int> commands) {
    std::cout << "[>] ";

    std::string cin{};
    std::getline(std::cin, cin);

	std::istringstream stream(cin);
	std::vector<std::string> arguments;
	std::string argument;

    while (stream >> argument) {
        arguments.push_back(argument);
    }

    if (commands.find(arguments[0]) != commands.end()) {
		switch (commands[arguments[0]]) {
		case 1:
			std::cout << "\n";
			std::cout << R"([H] 1. help)" << std::endl;
			std::cout << R"([H] 2. listpid)" << std::endl;
			std::cout << R"([H] 3. inject <"path/to/dll"> <procesname.exe>)" << std::endl;
			std::cout << R"([H] 4. exit)" << std::endl;
			std::cout << "\n";
			break;
		case 2:
			std::cout << "\n";
			list_processids();
			std::cout << "\n";
			break;
		case 3:
			if (arguments.size() == 3) {
				std::cout << "[+] Attempting injection\n";
				LPCSTR fake_dll_path = arguments[1].c_str();
				LPCSTR dll_path = get_full_path(fake_dll_path);

				DWORD process_id = get_process_id(arguments[2]);

				if (inject(dll_path, process_id) == true) {
					std::cout << "[+] injection was successful\n\n";
				} else {
					std::cout << "[+] injection failed\n\n";
				}
			} else {
				std::cout << "[-] type help for syntax\n\n";
			}
			break;
		case 4:
			break;
		}
    }
}

int main() { 
    std::unordered_map<std::string, int> commands = {
        {"help", 1},
        {"listpid", 2},
        {"inject", 3},
        {"exit", 4},
    };

    while (1) {
        command_handler(commands);
        Sleep(1);
    }

    return 0;
}
