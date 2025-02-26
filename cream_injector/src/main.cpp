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

bool inject(LPCSTR dll_path, DWORD process_id) {
    bool status = false;

    const HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
    if (handle == 0) { std::cout << "[-] Failed to obtain handle\n\n"; status = false; return status; }

    std::cout << "[+] Attempting to allocate memory\n";
    const LPVOID allocated = VirtualAllocEx(handle, 0, strlen(dll_path) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (allocated == 0) { std::cout << "[-] Failed to allocate memory\n\n"; status = false; CloseHandle(handle); return status; }
    std::cout << "[+] Successfully allocated memory\n";

    status = true;

    std::cout << "[+] Attempting to write memory\n";
    status = WriteProcessMemory(handle, allocated, dll_path, strlen(dll_path) + 1, 0);
    if (status == false) { std::cout << "[-] Failed to write memory\n\n"; CloseHandle(handle); return status; }
    std::cout << "[+] Successfully wrote to memory\n";

    const HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    if (kernel32 == 0) { std::cout << "[-] Failed to get HMODULE to kernel32.dll\n\n"; status = false; CloseHandle(handle); return status; }

    const FARPROC load_library = GetProcAddress(kernel32, "LoadLibraryA");
    if (load_library == 0) { std::cout << "[-] Failed to get LoadLibraryA() from kernel32.dll\n\n"; status = false; CloseHandle(handle); return status; }

    std::cout << "[+] Attempting to call LoadLibraryA\n";
    const HANDLE thread = CreateRemoteThread(handle, 0, 0, (LPTHREAD_START_ROUTINE)load_library, allocated, 0, 0);
    if (thread == 0) { status = false; std::cout << "[-] Failed to call LoadLibraryA\n\n";  CloseHandle(handle); return status; }
    std::cout << "[+] Successfully called LoadLibraryA\n";

    std::cout << "[+] Waiting for handle object\n";
    WaitForSingleObject(thread, INFINITE);
    std::cout << "[+] Finished waiting for handle object\n";
    
    std::cout << "[+] Attempting to free memory\n";
    status = VirtualFreeEx(handle, allocated, 0, MEM_RELEASE);
    if (status == false) { std::cout << "[-] Failed to free memory\n\n"; CloseHandle(handle); return status; }

    if (handle || thread) {
		CloseHandle(thread);
		CloseHandle(handle);
    }

    return status;
}

void command_handler(std::unordered_map<std::string, int> commands) {
    std::cout << "> ";

    std::string cin{};
    std::getline(std::cin, cin);

	std::istringstream stream(cin);
	std::vector<std::string> arguments;
	std::string argument;

	std::vector<std::string> help { R"(
[H] help
[H] listpid
[H] inject <path/to/dll> <processname.exe>
[H] exit
	)" };

    while (stream >> argument) {
        arguments.push_back(argument);
    }

    if (commands.find(arguments[0]) != commands.end()) {
		switch (commands[arguments[0]]) {
		case 1:
            for (auto line : help) {
                std::cout << line << "\n";
            }
			break;
		case 2:
			std::cout << "\n";
			list_processids();
			std::cout << "\n";
			break;
		case 3:
			std::cout << "\n";
			if (arguments.size() == 3) {
				LPCSTR fake_dll_path = arguments[1].c_str();
				LPCSTR dll_path = get_full_path(fake_dll_path);
                if (!file_exists(dll_path)) { std::cout << "[-] DLL does not exist\n"; break; }

				DWORD process_id = get_process_id(arguments[2]);

                std::cout << "\n[*] Full Path of DLL: " << dll_path << "\n";
                std::cout << "[*] Process ID of " << arguments[2] << ": " << process_id << "\n\n";
				if (inject(dll_path, process_id) == true) {
					std::cout << "[+] Injection was successful\n\n";
				} else {
					std::cout << "[+] Failed to inject into process\n\n";
				}
			} else {
				std::cout << "[-] type help for syntax\n\n";
			}
			break;
		case 4:
            exit(0);
			break;
        default:
            std::cout << "[-] type help\n\n";
            break;
		}
    }
}

int main() { 
    std::vector<std::string> intro{ R"(
|--------------------------------------------------------------------------|
|                                    _         _           __              | 
|    _____________  ____ _____ ___  (_)___    (_)__  _____/ /_____  _____  |
|   / ___/ ___/ _ \/ __ `/ __ `__ \/ / __ \  / / _ \/ ___/ __/ __ \/ ___/  |
|  / /__/ /  /  __/ /_/ / / / / / / / / / / / /  __/ /__/ /_/ /_/ / /      |
|  \___/_/   \___/\__,_/_/ /_/ /_/_/_/ /_/_/ /\___/\___/\__/\____/_/       |
|                                       /___/                              |
|--------------------------------------------------------------------------|
                   > help for a list of commands <
	)" };

    for (auto line : intro) {
        std::cout << line << "\n";
    }

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
