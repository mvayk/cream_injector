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

bool inject(LPCSTR dll_path) {
    bool status = false;

    return status;
}

std::vector<std::string> split_args(const std::string& input) {
    std::vector<std::string> args; std::istringstream stream(input); std::string argument;
    
    while (std::getline(stream, argument, ' ')) {
        if (!input.empty()) {
            args.push_back(argument);
        }
    }

    return args;
}

void command_handler(std::unordered_map<std::string, int> commands) {
    std::cout << "[>] ";

    std::string cin{};
    std::getline(std::cin, cin);

    std::vector<std::string> arguments = split_args(cin);

    switch (commands[cin]) {
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
        if (arguments.size() < 3) {
            std::cout << "[!] Missing arguments for injection. Usage: inject <path/to/dll> <processname.exe>\n";
        } else {
            LPCSTR dll_path = arguments[1].c_str();
            DWORD process_id = get_process_id(arguments[2]);
        }
		std::cout << "\n";
        std::cout << "[+] attempting injection\n";
		std::cout << "\n";
        break;
    case 4:
        break;
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
