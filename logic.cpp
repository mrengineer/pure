#include "mongoose.h"
#include <string>
#include <map>
#include <vector>

extern std::map<std::string, struct mg_connection *> client_map;
extern std::vector<struct mg_connection *> sse_connections;

std::string handle_command(const std::string& json_str) {
    // Простой парсинг для ping
    if (json_str.find("\"command\":\"ping\"") != std::string::npos) {
        size_t pos = json_str.find("\"client_id\":\"");
        if (pos != std::string::npos) {
            pos += 13;
            size_t end = json_str.find("\"", pos);
            std::string client_id = json_str.substr(pos, end - pos);
            return "{\"dom_id\":\"pong_span\",\"payload\":\"Pong\",\"client_id\":\"" + client_id + "\"}";
        }
    }
    // Для других команд можно расширить
    return "";
}