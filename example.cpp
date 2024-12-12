#include "./include/DesktopDuplication.hpp"

#include <iostream>
#include <conio.h>

void drawMain() {
    std::cout << "Desktop Duplication Example" << std::endl;

    std::cout << "1. Choose Output" << std::endl;
    std::cout << "\nq: Quit" << std::endl;

    std::cout << "\nChoose an option: ";
}

int main() {
    bool exit = false;

    while (!exit) {
        drawMain();

        char input = _getch();
		system("cls");
        switch (input) {
            case '1': {
                DesktopDuplication::DuplMan desktopDupl = DesktopDuplication::Singleton<DesktopDuplication::DuplMan>::Instance();
                DesktopDuplication::ChooseOutput();
                break;
            }
            case 'q': {
                exit = true;
                break;
            }
            default: {
                std::cout << "Invalid option. Please choose a valid option." << std::endl;
                std::cout << "Press any key to continue..." << std::endl;
                _getch();
                break;
            }
        }
    }

    return 0;
}