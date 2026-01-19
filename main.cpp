#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include <map>
#include <set>
#include <variant>
#include <mutex>
#include "webui.hpp"

/**
 * --- БЛОК КРОСС-ПЛАТФОРМЕННОСТИ ---
 */
#ifdef USE_PIGPIO
    #include <pigpio.h>
#else
    inline int gpioInitialise() { return 0; }
    inline void gpioTerminate() {}
#endif

// Контекст клиента: храним ID видимых элементов
struct ClientContext {
    std::set<std::string> visible_ids;
    std::mutex mtx;
    ClientContext() = default;
};

// Глобальные параметры системы
std::map<std::string, std::string> global_params = {
    {"fan_speed", "85"},
    {"power_on", "true"},
    {"station_name", "RPi-Zero-2026"}
};

std::map<size_t, ClientContext> clients;
std::mutex global_mtx;

// Формирование JS для обновления значений в UI
std::string make_val_js(const std::string& id, const std::string& val) {
    return "set_ui_value('" + id + "', '" + val + "');";
}

// Обработка видимости (Pub/Sub)
void handle_visibility(webui::window::event* e) {
    std::string data = e->get_string();
    size_t sep = data.find(':');
    if (sep == std::string::npos) return;
    
    std::string dom_id = data.substr(0, sep);
    bool is_visible = (data.substr(sep + 1) == "1");

    std::lock_guard<std::mutex> lock(global_mtx);
    if (clients.count(e->client_id)) {
        auto& ctx = clients[e->client_id];
        std::lock_guard<std::mutex> c_lock(ctx.mtx);
        if (is_visible) {
            if (ctx.visible_ids.insert(dom_id).second)
                std::cout << "[SUB] Client " << e->client_id << " -> " << dom_id << std::endl;
        } else {
            if (ctx.visible_ids.erase(dom_id))
                std::cout << "[UNSUB] Client " << e->client_id << " <- " << dom_id << std::endl;
        }
    }
}

// Обработка изменений в интерфейсе
void handle_change(webui::window::event* e) {
    std::string data = e->get_string();
    size_t sep = data.find(':');
    if (sep != std::string::npos) {
        std::string id = data.substr(0, sep);
        std::string val = data.substr(sep + 1);
        
        std::lock_guard<std::mutex> lock(global_mtx);
        global_params[id] = val; 
        std::cout << "[PARAM] Client " << e->client_id << " changed " << id << " to " << val << std::endl;
        
        // Синхронизация изменений между всеми вкладками
        for (auto& [cid, ctx] : clients) {
            if (cid != e->client_id) {
                // Прямой вызов C-API для адресной рассылки
                webui_run_client(reinterpret_cast<webui_event_t*>(e), make_val_js(id, val).c_str());
            }
        }
    }
}

// Управление жизненным циклом соединений
void event_common(webui::window::event* e) {
    std::lock_guard<std::mutex> lock(global_mtx);
    if (e->event_type == WEBUI_EVENT_CONNECTED) {
        clients.emplace(std::piecewise_construct, std::forward_as_tuple(e->client_id), std::forward_as_tuple());
        std::cout << "[CONN] Client " << e->client_id << " connected" << std::endl;
        
        // Инициализация параметров для новой вкладки
        for (auto const& [id, val] : global_params) {
            webui_run_client(reinterpret_cast<webui_event_t*>(e), make_val_js(id, val).c_str());
        }
    } else if (e->event_type == WEBUI_EVENT_DISCONNECTED) {
        clients.erase(e->client_id);
        std::cout << "[DISCONN] Client " << e->client_id << " disconnected" << std::endl;
    }
}

int main() {
    std::srand(std::time(nullptr));
    gpioInitialise();
    
    webui::window win;
    win.set_port(8081);
    webui::set_config(webui_config::multi_client, true);

    win.bind("", event_common);
    win.bind("set_visible", handle_visibility);
    win.bind("handle_change", handle_change);

    // Фоновый поток обновления
    std::thread([&win]() {
        while (true) {
            // Общее время (шлем всем)
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::stringstream ss; ss << std::put_time(std::localtime(&now), "%H:%M:%S");
            std::string time_js = "update_element(JSON.stringify({dom_id:'timer-display', payload:'" + ss.str() + "'}));";
            win.run(time_js);

            {
                std::lock_guard<std::mutex> lock(global_mtx);
                for (auto& [cid, ctx] : clients) {
                    std::lock_guard<std::mutex> c_lock(ctx.mtx);
                    for (const std::string& id : ctx.visible_ids) {
                        int sensor_val = std::rand() % 1000;
                        std::string packet = "update_element(JSON.stringify({dom_id:'" + id + "', payload:'" + std::to_string(sensor_val) + "'}));";
                        
                        // В многопоточном режиме для конкретного клиента используем win.run()
                        // В v2.5+ win.run(script) отправляет всем, 
                        // Для конкретного cid используем webui_run_client
                        // Примечание: Для фонового потока без объекта события 
                        // используется webui_script_client (если доступен в C)
                    }
                }
            }
            win.run("update_graph(" + std::to_string(std::rand() % 50 + 20) + ");");
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }).detach();

#ifdef USE_PIGPIO
    win.show_browser("index.html", 0); 
#else
    win.show("index.html");
#endif
    webui::wait();
    gpioTerminate();
    return 0;
}
