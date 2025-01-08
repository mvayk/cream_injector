#include <cstring>
#include <iostream>
#include <processthreadsapi.h>
#include <windows.h>
#include <TlHelp32.h>
#include <string>
#include <vector>

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

bool inject(LPCSTR dll_path) {
    bool status = false;

    return status;
}

/* git is broken */

void command_handler(std::vector<std::string> commands) {
    std::cout << "[>] ";

    std::string cin {};
    std::cin >> cin;

    for (auto & input : commands) {
        if (input == commands[1]) {
            std::cout << "\n";

            std::cout << R"([H] 1. help)" << std::endl;
            std::cout << R"([H] 2. listpid)" << std::endl;
            std::cout << R"([H] 3. inject <"path/to/dll"> <procesname.exe>)" << std::endl;
            std::cout << R"([H] 4. exit)" << std::endl;
        } else if (input == commands[2]) {
            std::cout << "\n";

            list_processids();
        } else if (input == commands[3]) {
        }
    }
}

int main() { 
    std::vector<std::string> commands = { "help", "listpid", "inject", "exit" };

    while (1) {
        command_handler(commands);
    }

    return 0;
}
