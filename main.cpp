#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <map>
#include <variant>
#include "webui.hpp"
#include <pigpio.h>
#include <fstream>


// Типы данных для нашей системы управления
enum class ParamType { INT, FLOAT, BOOL, ENUM, STRING };

struct ControlParam {
    ParamType type;
    std::variant<int, float, bool, std::string> value;
    int gpio_pin;
    std::string description;
    void (*apply_func)(const std::string& id, const ControlParam& p);
};

// --- Функции-обработчики для железа ---
float get_cpu_temp() {
    float temp = 0.0;
    std::ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
    if (tempFile.is_open()) {
        int raw_temp;
        tempFile >> raw_temp;
        temp = raw_temp / 1000.0f; // Значение хранится в миллиградусах
        tempFile.close();
    }
    return temp;
}

void apply_pwm(const std::string& id, const ControlParam& p) {
    int val = std::get<int>(p.value);
    // Для GPIO 18 используем Hardware PWM, для остальных обычный
    if (p.gpio_pin == 18) {
        gpioHardwarePWM(p.gpio_pin, 100, val * 10000);
    } else {
        gpioPWM(p.gpio_pin, (int)(val * 2.55));
    }
    std::cout << "[HARDWARE] " << id << " set to " << val << "%" << std::endl;
}

void apply_generic_log(const std::string& id, const ControlParam& p) {
    std::cout << "[LOG] Параметр '" << id << "' изменен. Тип: " << (int)p.type << std::endl;
}

// --- РЕЕСТР ПАРАМЕТРОВ (Добавлять новые параметры здесь) ---
std::map<std::string, ControlParam> registry = {
    // ID в HTML         Тип             Дефолт    Pin    Описание             Callback
    {"fan_speed",    {ParamType::INT,    85,       18,   "Вентилятор %",      apply_pwm}},
    {"target_temp",  {ParamType::FLOAT,  24.5f,    0,    "Порог темп.",       apply_generic_log}},
    {"system_ready", {ParamType::BOOL,   true,     0,    "Статус системы",    apply_generic_log}},
    {"work_mode",    {ParamType::ENUM,   1,        0,    "Режим работы",      apply_generic_log}},
    {"station_name", {ParamType::STRING, std::string("RPi-2026"), 0, "Имя",   apply_generic_log}}
};

// Универсальный JS помощник
std::string make_js_call(const std::string& dom_id, const std::string& payload) {
    return "update_element(JSON.stringify({dom_id: '" + dom_id + "', payload: '" + payload + "'}));";
}

// Рассылка текущих значений всем клиентам
void sync_all_controls(webui::window::event* e) {
    for (auto const& [id, p] : registry) {
        std::string val_str;
        if (std::holds_alternative<int>(p.value)) val_str = std::to_string(std::get<int>(p.value));
        else if (std::holds_alternative<float>(p.value)) {
            std::stringstream ss; ss << std::fixed << std::setprecision(1) << std::get<float>(p.value);
            val_str = ss.str();
        }
        else if (std::holds_alternative<bool>(p.value)) val_str = std::get<bool>(p.value) ? "true" : "false";
        else if (std::holds_alternative<std::string>(p.value)) val_str = std::get<std::string>(p.value);

        e->run_client("set_ui_value('" + id + "', '" + val_str + "');");
    }
}

void handle_universal_update(webui::window::event* e) {
    std::string data = e->get_string(); // "id:value"
    size_t sep = data.find(':');
    if (sep == std::string::npos) return;

    std::string id = data.substr(0, sep);
    std::string val_str = data.substr(sep + 1);

    if (registry.count(id)) {
        auto& p = registry[id];
        try {
            if (p.type == ParamType::INT || p.type == ParamType::ENUM) p.value = std::stoi(val_str);
            else if (p.type == ParamType::FLOAT) p.value = std::stof(val_str);
            else if (p.type == ParamType::BOOL) p.value = (val_str == "true");
            else if (p.type == ParamType::STRING) p.value = val_str;

            if (p.apply_func) p.apply_func(id, p);
        } catch (...) { std::cerr << "Error parsing: " << id << std::endl; }
    }
}

void event_common(webui::window::event* e) {
    if (e->event_type == WEBUI_EVENT_CONNECTED) {
        sync_all_controls(e);
    }
}

int main() {
    if (gpioInitialise() < 0) {
        std::cerr << "pigpio initialisation failed." << std::endl;
        return 1;
    }


    webui::window my_window;
    my_window.set_public(true);
    my_window.set_port(8081);
    webui::set_timeout(0);
    webui::set_config(webui_config::multi_client, true);
    


    my_window.bind("", event_common);
    my_window.bind("handle_update", handle_universal_update);

    // Фоновый поток для времени (1с) и графика температуры (5с)
    std::thread([&my_window]() {
        int counter = 0;
        while (true) {
            // 1. Обновляем время каждую секунду
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::stringstream ss; 
            ss << std::put_time(std::localtime(&now), "%H:%M:%S");
            my_window.run(make_js_call("timer-display", ss.str()));

            // 2. Обновляем график только раз в 5 секунд
            if (counter % 5 == 0) {
                float current_temp = get_cpu_temp();
                // Форматируем до 1 знака после запятой
                std::stringstream temp_ss;
                temp_ss << std::fixed << std::setprecision(1) << current_temp;
                
                my_window.run("update_graph(" + temp_ss.str() + ");");
                
                // Опционально: выводим в консоль для отладки
                // std::cout << "[INFO] CPU Temp: " << temp_ss.str() << "°C" << std::endl;
            }

            counter++;
            if (counter >= 3600) counter = 0; // Сброс счетчика раз в час
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }).detach();

    
    // Запуск в режиме сервера БЕЗ открытия браузера
    if (!my_window.show_browser("index.html", 0)) {
        printf("Failed to start server\n");
        return 1;
    }

    // Печать URL: преобразуем string_view в std::string
    std::string url(my_window.get_url());
    printf("Server started. Access it at: http://<ВАШ_IP>:8081 (Internal: %s)\n", url.c_str());

    webui::wait();
    gpioTerminate();
    return 0;
}
