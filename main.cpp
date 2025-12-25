#include <iostream>
#include <string>
#include "webui.hpp"
#include <filesystem> // C++17
namespace fs = std::filesystem;


int main() {
    webui::window my_window;

    // 1. Настройки порта и доступа
    my_window.set_public(true); // Теперь включаем для планшета
    my_window.set_port(8081);
    webui::set_config(folder_monitor, true);
    webui::set_config(multi_client, true);
    

    // 2. Обработчик событий (мониторинг подключений)
    my_window.bind("", [](webui::window::event* e) {
        if (e->event_type == WEBUI_EVENT_CONNECTED) {
            std::cout << "[СЕРВЕР] Клиент подключился!" << std::endl;
        } else if (e->event_type == WEBUI_EVENT_DISCONNECTED) {
            std::cout << "[СЕРВЕР] Клиент отключился." << std::endl;
        }
    });

    // Получаем абсолютный путь к файлу рядом с бинарником
    std::string full_path = (fs::current_path() / "index.html").string();
    std::cout << "Попытка открыть: " << full_path << std::endl;

    if (!my_window.show_browser("index.html", 0)) {  //0 - браузер
        std::cout << "Ошибка: Не удалось открыть файл или порт занят." << std::endl;
        return 1;
    }

    std::cout << "=======================================" << std::endl;
    std::cout << "Сервер запущен на порту 8081" << std::endl;
    std::cout << "=======================================" << std::endl;

    webui::wait(); 
    return 0;
}
