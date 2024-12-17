#include "./include/DesktopDuplication.hpp"

#include <iostream>
#include <conio.h>

void drawMain() {
    std::cout << "Desktop Duplication Example" << std::endl;

    std::cout << "1. Choose Output" << std::endl;
    std::cout << "2. Start Duplication" << std::endl;
    std::cout << "3. Take Screenshot" << std::endl;
    std::cout << "4. Start DuplicationLoop" << std::endl;
    std::cout << "\nq: Quit" << std::endl;

    std::cout << "\nChoose an option: ";
}

void MonitorThread();

int main() {
    bool exit = false;

    while (!exit) {
        drawMain();

        char input = _getch();
		system("cls");
        switch (input) {
            case '1': {
                DesktopDuplication::Duplication& dupl = DesktopDuplication::Singleton<DesktopDuplication::Duplication>::Instance();
                DesktopDuplication::ChooseOutput();
                break;
            }
            case '2': {
                DesktopDuplication::Duplication& dupl = DesktopDuplication::Singleton<DesktopDuplication::Duplication>::Instance();
                dupl.InitDuplication();
                break;
            }
            case '3': {
                DesktopDuplication::Duplication& dupl = DesktopDuplication::Singleton<DesktopDuplication::Duplication>::Instance();
                dupl.SaveFrame("./");
                break;
            }
            case '4': {
                DesktopDuplication::Duplication& dupl = DesktopDuplication::Singleton<DesktopDuplication::Duplication>::Instance();
                DesktopDuplication::DuplicationThread& thread = DesktopDuplication::Singleton<DesktopDuplication::DuplicationThread>::Instance();
                thread.SetDuplication(&dupl);
                bool start = thread.Start();
                if (!start) abort();
                MonitorThread();
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

void MonitorThread() {
    DesktopDuplication::DuplicationThread& thread = DesktopDuplication::Singleton<DesktopDuplication::DuplicationThread>::Instance();
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime = std::chrono::high_resolution_clock::now();
    int frameCount = 0;
    thread.RegisterTelemetry(&frameCount);
    std::cout << "Press 'q' to stop the thread." << std::endl;
    while (true) {
        if (_kbhit()) {
            char input = _getch();
            if (input == 'q') {
                thread.Stop();
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = currentTime - lastTime;

        if (elapsed.count() >= 1.0) {
            std::cout << "                                                            " << "\r" << std::flush;
            std::cout << "FPS: " << frameCount << "\r" << std::flush;
            frameCount = 0;
            lastTime = currentTime;
        }
    }
}