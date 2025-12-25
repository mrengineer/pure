# Raspberry Pi Web Server / Веб-сервер для Raspberry Pi

## English

Simple web server in C++ for Raspberry Pi using the Mongoose library. Supports Server-Sent Events (SSE) for real-time time updates and command handling via POST requests in JSON.

### Features

- Display status and current time with real-time updates via SSE.
- "Ping" button for testing commands (sends POST request, receives "Pong" response to the specific client).
- Universal command handling in `logic.cpp` with JSON input/output.
- JSON data exchange for commands.
- Support for static files (images, JS) served from `static/` directory.
- Multi-client support with unique client IDs.

### Build and Run

#### Requirements

- CMake
- C++ compiler (g++)
- Raspberry Pi (or POSIX-compatible Linux)

#### Build

```bash
mkdir build
cd build
cmake ..
make
```

#### Run

```bash
./server
```

The server will start on port 8000. Open http://localhost:8000 in your browser.

### Project Structure

- `main.cpp`: Main server code, HTTP/SSE handling, connection management.
- `logic.cpp`: Command logic processing.
- `mongoose.h` / `mongoose.c`: Mongoose library for networking.
- `CMakeLists.txt`: Build configuration.
- `static/index.html`: Main HTML page with JS for SSE and commands.
- `.gitignore`: Ignored files.

### Usage

- The page displays status ("Status: Working") and time (updates every 10 seconds via SSE).
- Click "Ping" to send a command: POST to `/command` with JSON `{"command": "ping", "client_id": "unique_id"}`.
- Response: JSON `{"dom_id": "pong_span", "payload": "Pong", "client_id": "unique_id"}`, updates the span.
- For broadcasting to all clients, set `client_id` empty in logic.

### API

- `GET /`: Serves `static/index.html`.
- `GET /static/*`: Serves static files.
- `GET /sse?client_id=...`: SSE endpoint for real-time updates.
- `POST /command`: JSON command processing.

## Русский

Простой веб-сервер на C++ для Raspberry Pi с использованием библиотеки Mongoose. Поддерживает Server-Sent Events (SSE) для обновления времени в реальном времени и обработку команд через POST-запросы в JSON.

### Возможности

- Отображение статуса и текущего времени с обновлениями в реальном времени через SSE.
- Кнопка "Ping" для тестирования команд (отправляет POST-запрос, получает ответ "Pong" конкретному клиенту).
- Универсальная обработка команд в `logic.cpp` с JSON входом/выходом.
- Обмен данными в JSON для команд.
- Поддержка статических файлов (картинки, JS) из директории `static/`.
- Поддержка нескольких клиентов с уникальными ID клиентов.

### Сборка и запуск

#### Требования

- CMake
- Компилятор C++ (g++)
- Raspberry Pi (или POSIX-совместимый Linux)

#### Сборка

```bash
mkdir build
cd build
cmake ..
make
```

#### Запуск

```bash
./server
```

Сервер запустится на порту 8000. Откройте http://localhost:8000 в браузере.

### Структура проекта

- `main.cpp`: Основной код сервера, обработка HTTP/SSE, управление соединениями.
- `logic.cpp`: Логика обработки команд.
- `mongoose.h` / `mongoose.c`: Библиотека Mongoose для сетевых операций.
- `CMakeLists.txt`: Конфигурация сборки.
- `static/index.html`: Главная HTML-страница с JS для SSE и команд.
- `.gitignore`: Игнорируемые файлы.

### Использование

- Страница отображает статус ("Статус: Работает") и время (обновляется каждые 10 секунд через SSE).
- Нажмите "Ping" для отправки команды: POST на `/command` с JSON `{"command": "ping", "client_id": "unique_id"}`.
- Ответ: JSON `{"dom_id": "pong_span", "payload": "Pong", "client_id": "unique_id"}`, обновляет span.
- Для рассылки всем клиентам установите `client_id` пустым в логике.

### API

- `GET /`: Отдает `static/index.html`.
- `GET /static/*`: Отдает статические файлы.
- `GET /sse?client_id=...`: SSE endpoint для обновлений в реальном времени.
- `POST /command`: Обработка команд в JSON.
