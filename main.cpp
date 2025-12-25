#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include "webui.hpp"

// Вспомогательная функция для формирования JS-вызова
// Генерирует строку вида: update_element('{"dom_id":"...", "payload":"..."}')
std::string make_js_call(const std::string& dom_id, const std::string& payload) {
    return "update_element(JSON.stringify({dom_id: '" + dom_id + "', payload: '" + payload + "'}));";
}

// 1. Мониторинг подключений
void event_common(webui::window::event* e) {
    if (e->event_type == WEBUI_EVENT_CONNECTED) {
        std::cout << "[ПОДКЛЮЧЕНИЕ] ID: " << e->client_id << std::endl;
        
        // Отправляем ID конкретному клиенту
        std::string js = make_js_call("my-id-display", std::to_string(e->client_id));
        e->run_client(js);
    }
}

// 2. Обработчик команд
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
    webui::window my_window;
    
    my_window.set_public(true);
    my_window.set_port(8081);
    webui::set_config(webui_config::multi_client, true);

    // Привязываем функции (без захвата контекста в лямбду)
    my_window.bind("", event_common);
    my_window.bind("handle_request", handle_request);

    // 3. Поток рассылки времени
    std::thread timer_thread([&my_window]() {
        while (true) {
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::stringstream ss;
            ss << std::put_time(std::localtime(&now), "%H:%M:%S");
            
            // Рассылка всем
            my_window.run(make_js_call("timer-display", ss.str()));
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    timer_thread.detach();

    my_window.show("index.html");
    webui::wait();
    return 0;
}
