cmake_minimum_required(VERSION 3.10)

project(pure CXX)

# Настройки стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- ОБЩИЕ НАСТРОЙКИ ---
set(WEBUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(WEBUI_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# 1. WebUI нужен для обеих платформ
add_subdirectory(external/webui)


# Aддитивные флаги компиляции для отладки
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 2. Условная настройка PIGPIO (только для ARM)
# В докере для Pi Zero переменная CMAKE_SYSTEM_PROCESSOR будет "arm"
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    message(STATUS ">>> ARM DETECTED: Including PIGPIO")
    
    # Исправление для библиотеки RT в старых версиях pigpio
    set(RT_LIBRARIES "-lrt" CACHE STRING "" FORCE)
    set(RT_FOUND TRUE CACHE BOOL "" FORCE)
    
    add_subdirectory(external/pigpio)
    set(PIGPIO_LIBRARIES pigpio rt)
    add_definitions(-DUSE_PIGPIO) # Прокидываем макрос в код
else()
    message(STATUS ">>> X86 DETECTED: Skipping PIGPIO (using MOCKS)")
    set(PIGPIO_LIBRARIES "")
endif()

# 3. Создание исполняемого файла
add_executable(${PROJECT_NAME} main.cpp)

# 4. Пути к заголовочным файлам
target_include_directories(${PROJECT_NAME} PRIVATE 
    external/webui/include
)

# Если ARM, добавляем заголовки pigpio
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    target_include_directories(${PROJECT_NAME} PRIVATE external/pigpio)
endif()

# 5. Флаги линковки (для Pi Zero важна статика)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
endif()

find_package(Threads REQUIRED)

# 6. Линковка библиотек
target_link_libraries(${PROJECT_NAME} PRIVATE 
    webui
    ${PIGPIO_LIBRARIES}
    Threads::Threads
    ${CMAKE_DL_LIBS}
)

# 7. Копирование ресурсов (index.html, bootstrap.min.css, smoothie.js)
set(RESOURCES 
    "index.html" 
    "bootstrap.min.css" 
    "smoothie.js"
)

foreach(RES ${RESOURCES})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/${RES}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${RES}"
    )
endforeach()
