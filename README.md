# Air Purifier Control System (2026)

Проект системы управления на базе **Raspberry Pi Zero** и **WebUI**. 
Реализует высокопроизводительный интерфейс с поддержкой сотен динамических элементов.

## Основная концепция
Чтобы обеспечить плавную работу на слабом железе (ARMv6, 512MB RAM), проект использует **Selective Data Streaming (Pub/Sub)**:
- Сервер не транслирует все 300 параметров одновременно.
- Браузер сообщает серверу, какие элементы видны на экране в данный момент (используя `IntersectionObserver`).
- Сервер отправляет обновления только для видимых элементов индивидуально для каждой открытой вкладки.

## Особенности реализации
- Multi-client: Каждая вкладка браузера имеет свой независимый список видимых элементов.
- Static Linking: Бинарник для Pi Zero собирается со статическими libgcc и libstdc++ для работы на любом дистрибутиве (Raspberry Pi OS Lite и др.).
- Zero Latency: Использование нативных WebSocket через WebUI обеспечивает минимальный отклик при управлении GPIO.


## Структура проекта
- `main.cpp`: Логика сервера, управление GPIO и менеджер подписок клиентов.
- `index.html`: Интерфейс на Bootstrap 5 с логикой отслеживания видимости.
- `external/`: Субмодули (WebUI, pigpio).
- `docker/`: Окружение для кросс-компиляции под Raspberry Pi Zero.

## Сборка
Проект поддерживает двойную сборку через системный CMake (требуется версия 3.10+):

### 1. Локальная (Ubuntu x86_64)
Используется для разработки и тестов интерфейса. Библиотека `pigpio` заменяется заглушками.
```bash
# Air Purifier Control System (2026)

Проект системы управления на базе Raspberry Pi Zero и WebUI. Решение оптимизировано под малые ресурсы и использует селективную потоковую передачу данных (Pub/Sub) для минимизации нагрузки.

**Архитектура**

- Core (C++): `main.cpp` — запускает `webui::window` (HTTP + WebSocket), держит `global_params` и карту клиентов `clients`, где для каждого клиента хранится `ClientContext` (набор видимых DOM id и при необходимости `connection_id`). Фоновый поток генерирует значения датчиков и отправляет их только видимым элементам.
- WebUI (external/webui): библиотека обеспечивает транспорт (WebSocket), JS<->C мост и механизмы идентификации клиентов (cookies и `connection_id`). Смотрите `external/webui/include/webui.h` и `external/webui/include/webui.hpp`.
- Client (браузер): `index.html` + `webui.js` — на стороне клиента реализован `IntersectionObserver`, который при появлении/скрытии элементов шлёт `set_visible` события на сервер; входящие обновления обрабатываются функцией `update_element()`.

Дизайн-заметки

- Selective Data Streaming: клиент сообщает серверу лишь id видимых элементов; сервер отправляет только нужные обновления — это критично для устройств с ограниченными ресурсами.
- Идентификация клиентов: библиотека может применять cookies (`use_cookies`) или опираться на WebSocket `connection_id`. Вкладки в одном профиле браузера разделяют cookie; если нужно адресовать конкретную вкладку — используйте `connection_id`.

Как добавлять новые параметры

1) Сервер — объявите параметр в `global_params` (в `main.cpp`):

```cpp
std::map<std::string, std::string> global_params = {
	{"fan_speed", "85"},
	{"power_on", "true"},
	{"station_name", "RPi-Zero-2026"},
	{"heater_enabled", "false"} // новый параметр
};
```

2) Клиент — добавьте элемент в `index.html` с соответствующим `id` (например `heater-enabled`) и убедитесь, что `update_element()` умеет обновлять этот элемент (обычно это универсальная функция, уже присутствующая в шаблоне).

3) Серверная логика: в `handle_change(webui::window::event* e)` изменения приходят в виде `id:val`. В текущей реализации `global_params[id] = val;` и затем выполняется рассылка обновления другим клиентам.

Пример серверного принудительного обновления и рассылки всем клиентам:

```cpp
global_params["heater_enabled"] = "true";
win.run("update_element(JSON.stringify({dom_id:'heater-enabled', payload:'true'}));");
```

Как рассылать данные

- Всем клиентам (broadcast): `webui::window::run()` или `webui_run(window, script)` — отправляет скрипт всем подключённым WebSocket соединениям для данного окна:

```cpp
e->get_window().run("set_ui_value('fan_speed','90');");
// или
win.run("set_ui_value('fan_speed','90');");
```

- Конкретному клиенту (одно соединение): `webui_run_client()` / `event->run_client()` — адресуют скрипт конкретному `connection_id`. Пример создания временного события:

```cpp
// из обработчика события (посылает в исходный клиент)
e->run_client("set_ui_value('power_on','false');");

// адресная рассылка к конкретному connection_id (серверная сторона)
webui_event_t tmp = *reinterpret_cast<webui_event_t*>(e);
tmp.connection_id = target_connection_id; // индекс слота WS
webui_run_client(&tmp, "set_ui_value('fan_speed','60');");
```

Отправка всем, кроме источника

Если требуется обновить всех клиентов, кроме того, кто инициировал изменение, можно итерировать карту `clients` и вызывать `webui_run_client()` для каждого `connection_id`, за исключением источника:

```cpp
std::lock_guard<std::mutex> lock(global_mtx);
for (auto& [cid, ctx] : clients) {
	if (cid == e->client_id) continue; // пропустить источник
	webui_event_t tmp = *reinterpret_cast<webui_event_t*>(e);
	tmp.connection_id = ctx.connection_id;
	webui_run_client(&tmp, make_val_js(id, val).c_str());
}
```

Практические замечания

- Если вы тестируете несколько вкладок в одном профиле браузера, они будут разделять cookie и могут получить одинаковый `client_id`. Для отдельной идентификации используйте разные `--user-data-dir` или инкогнито-профили, либо адресуйте по `connection_id`.
- `win.run()` — самый простой и надёжный способ синхронизировать UI у всех открытых вкладок.

Где смотреть в коде

- Основная логика: `main.cpp`
- WebUI C API: `external/webui/include/webui.h`
- WebUI C++ wrapper: `external/webui/include/webui.hpp`

Примеры запуска браузера в отдельных профилях (Linux, Chrome/Chromium):

```bash
google-chrome --user-data-dir=/tmp/profile1 'http://127.0.0.1:8081'
google-chrome --user-data-dir=/tmp/profile2 'http://127.0.0.1:8081'
```

Если хотите, могу добавить в репозиторий:

- расширенный пример в `index.html` с `IntersectionObserver`,
- утилиту запуска браузера с генерацией временных профилей для теста, или
- раздел с быстрыми командами отладки.
