# WebUI C++ Template / Шаблон GUI на WebUI для C++

Этот репозиторий — **готовый шаблон** для создания простого графического веб-интерфейса (GUI) на базе библиотеки **WebUI** для программ на C++.

WebUI позволяет запускать локальный веб-сервер и отображать HTML-интерфейс прямо в браузере, при этом вся логика остаётся на C++. Поддерживается множественное подключение клиентов, отправка данных конкретному клиенту или всем сразу.

This repository is a **ready-to-use template** for creating a simple web-based GUI using the **WebUI** library for C++ programs.  
All logic stays in C++, while the interface is displayed in the browser.

## Особенности / Features

- Мульти-клиент (несколько пользователей одновременно) / Multi-client support
- Обновление элементов DOM через JSON / DOM element updates via JSON
- Рассылка данных всем клиентам / Broadcasting to all clients
- Отправка сообщений конкретному клиенту / Sending messages to a specific client
- Живой график на SmoothieChart / Live chart using SmoothieChart
- Обновление времени сервера в реальном времени / Real-time server time updates
- Обработка событий подключения/отключения / Connection/disconnection events handling

## Структура проекта / Project structure
.
├── CMakeLists.txt          # Сборка через CMake
├── main.cpp                # Основной код приложения
├── index.html              # Веб-интерфейс
├── smoothie.js             # Библиотека для графика
└── ...                     # Другие файлы (webui.hpp и т.д.)



## Компиляция / Compilation

### Рекомендуемый способ: через CMake и make

```bash
mkdir build
cd build
cmake ..
make
./pure

