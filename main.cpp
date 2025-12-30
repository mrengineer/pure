
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include <cstdlib>   // для rand()
#include <ctime>     // для srand(time(NULL))
#include "webui.hpp"

// Вспомогательная функция для формирования JS-вызова
std::string make_js_call(const std::string& dom_id, const std::string& payload) {
    return "update_element(JSON.stringify({dom_id: '" + dom_id + "', payload: '" + payload + "'}));";
}

// Специальная функция для отправки значения на график
std::string make_graph_update(const std::string& value) {
    return "update_graph(" + value + ");";
}

// Общий обработчик подключения/отключения
void event_common(webui::window::event* e) {
    if (e->event_type == WEBUI_EVENT_CONNECTED) {
        std::cout << "[ПОДКЛЮЧЕНИЕ] ID: " << e->client_id << std::endl;

        std::string welcome = "Добро пожаловать! Ваш ID: " + std::to_string(e->client_id);
        e->run_client(make_js_call("main-status", welcome));
        e->run_client(make_js_call("my-id-display", std::to_string(e->client_id)));
    }
    else if (e->event_type == WEBUI_EVENT_DISCONNECTED) {
        std::cout << "[ОТКЛЮЧЕНИЕ] ID: " << e->client_id << std::endl;
    }
}

// Обработчик команд от клиента
void handle_request(webui::window::event* e) {
    std::string command = e->get_string();
    
    if (command == "ping") {
        std::string msg = "Понг для #" + std::to_string(e->client_id);
        e->run_client(make_js_call("main-status", msg));
    } 
    else if (command == "get_secret") {
        e->run_client(make_js_call("main-status", "Секретный код: 42"));
    }
}

int main() {
    printf("Program started...\n");
    fflush(stdout);
    
    
    std::srand(std::time(nullptr)); // инициализация rand

    webui::window my_window;
    
    my_window.set_public(true);
    my_window.set_port(8081);
    webui::set_config(webui_config::multi_client, true);

    my_window.bind("", event_common);
    my_window.bind("handle_request", handle_request);

    // Поток: рассылка времени + значений для графика
    std::thread updater_thread([&my_window]() {
        while (true) {
            // Текущее время
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::stringstream ss;
            ss << std::put_time(std::localtime(&now), "%H:%M:%S");
            my_window.run(make_js_call("timer-display", ss.str()));

            // Случайное значение для графика (0-100), как в старом main.c
            int graph_value = std::rand() % 101; // 0..100
            my_window.run(make_graph_update(std::to_string(graph_value)));

            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // можно менять частоту
        }
    });
    updater_thread.detach();

    // Запуск в режиме сервера БЕЗ открытия браузера
    if (!my_window.show_browser("index.html", 0)) {
        printf("Failed to start server\n");
        return 1;
    }

    // Печать URL: преобразуем string_view в std::string
    std::string url(my_window.get_url());
    printf("Server started. Access it at: http://<ВАШ_IP>:8081 (Internal: %s)\n", url.c_str());

    webui::wait();
    return 0;
}