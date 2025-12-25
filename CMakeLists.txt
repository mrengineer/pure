cmake_minimum_required(VERSION 3.10)
project(pure CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Добавляем субмодуль
# Если папка точно external/webui, CMake ее увидит после git submodule update
add_subdirectory(external/webui)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE external/webui/include)

# В WebUI таргет обычно называется просто "webui"
target_link_libraries(${PROJECT_NAME} PRIVATE webui)

# Запрещаем искать библиотеки в путях Snap
set(CMAKE_IGNORE_PATH "/snap/core20/current/lib/x86_64-linux-gnu")
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

# 3. Дополнительные зависимости для Linux
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    webui 
    Threads::Threads
    ${CMAKE_DL_LIBS}
)

# 4. Копирование файлов (улучшенный метод через generator expressions)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/index.html"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/index.html"
)
