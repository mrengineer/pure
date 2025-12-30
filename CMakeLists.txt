cmake_minimum_required(VERSION 3.10)

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)

project(pure CXX)

# Настройки стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ручное исправление ошибки pigpio с библиотекой RT
set(RT_LIBRARIES "-lrt" CACHE STRING "" FORCE)
set(RT_FOUND TRUE CACHE BOOL "" FORCE)

set(WEBUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # Отключаем примеры
set(WEBUI_BUILD_STATIC ON CACHE BOOL "" FORCE) # Включаем статическую сборку WebUI
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)  # Добавьте эту строку для pigpio


# 1. Настройка субмодулей
# Включаем сборку статической версии WebUI (важно для Pi Zero)
set(WEBUI_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # В pigpio отключаем сборку shared библиотек

# Добавляем папки субмодулей
add_subdirectory(external/webui)
add_subdirectory(external/pigpio)

# 2. Создание исполняемого файла
add_executable(${PROJECT_NAME} main.cpp)

# 3. Пути к заголовочным файлам (include)
target_include_directories(${PROJECT_NAME} PRIVATE 
    external/webui/include
    external/pigpio
)

# 4. Флаги компиляции и линковки
# -static-libgcc и -static-libstdc++ включают стандартные библиотеки внутрь файла
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")

# Поиск системных потоков (pthread)
find_package(Threads REQUIRED)

# 5. Линковка библиотек
# Используем имена целей (targets), которые создают субмодули
target_link_libraries(${PROJECT_NAME} PRIVATE 
    webui           # Пробуем основное имя
    pigpio
    Threads::Threads
    rt
    ${CMAKE_DL_LIBS}
)

# 6. Копирование интерфейса в папку с бинарником
# Чтобы index.html лежал рядом с файлом pure
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/index.html"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/index.html"
)
