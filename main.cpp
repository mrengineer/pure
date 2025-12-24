#include "mongoose.h"
#include <string>
#include <vector>
#include <map>
#include <time.h>
#include <unistd.h>
#include <algorithm>  // Для std::remove

std::vector<struct mg_connection *> ws_connections;
std::map<std::string, struct mg_connection *> client_map;

extern std::string handle_command(const std::string& json_str);

static void event_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_match(hm->uri, mg_str("/ws"), NULL)) {
            char client_id_buf[64] = {0};
            mg_http_get_var(&hm->query, "client_id", client_id_buf, sizeof(client_id_buf));
            if (client_id_buf[0]) {
                mg_ws_upgrade(c, hm, NULL);
                client_map[client_id_buf] = c;
                ws_connections.push_back(c);
            } else {
                mg_http_reply(c, 400, "Content-Type: text/plain\r\n", "Missing client_id");
            }
        } else {
            struct mg_http_serve_opts opts = {.root_dir = "static/"};
            mg_http_serve_dir(c, hm, &opts);
        }
    } 
    else if (ev == MG_EV_WS_MSG) {
        struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
        // Правильные поля для твоей версии Mongoose: data.buf и data.len
        std::string json_str(wm->data.buf, wm->data.len);
        std::string response = handle_command(json_str);
        if (!response.empty()) {
            // Парсим client_id из ответа
            size_t pos = response.find("\"client_id\":\"");
            std::string target_client_id;
            if (pos != std::string::npos) {
                pos += 13;
                size_t end = response.find("\"", pos);
                if (end != std::string::npos) {
                    target_client_id = response.substr(pos, end - pos);
                }
            }

            if (target_client_id.empty()) {
                // Broadcast всем
                for (auto conn : ws_connections) {
                    if (conn && !conn->is_closing) {
                        mg_ws_send(conn, response.c_str(), response.length(), WEBSOCKET_OP_TEXT);
                    }
                }
            } else {
                auto it = client_map.find(target_client_id);
                if (it != client_map.end() && it->second && !it->second->is_closing) {
                    mg_ws_send(it->second, response.c_str(), response.length(), WEBSOCKET_OP_TEXT);
                }
            }
        }
    } 
    else if (ev == MG_EV_CLOSE) {
        // Удаляем из списка и карты
        ws_connections.erase(std::remove(ws_connections.begin(), ws_connections.end(), c), ws_connections.end());
        for (auto it = client_map.begin(); it != client_map.end(); ) {
            if (it->second == c) {
                it = client_map.erase(it);
            } else {
                ++it;
            }
        }
    }
}

int main() {
    struct mg_mgr mgr;
    mg_log_set(MG_LL_NONE);
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:8000", event_handler, NULL);
    printf("Сервер запущен на порту 8000\n");

    time_t last_time_update = 0;

    for (;;) {
        mg_mgr_poll(&mgr, 500);

        // Отправка времени каждые 1 секунду
        time_t now = time(NULL);
        if (now - last_time_update >= 1) {
            char buf[64];
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
            std::string time_json = "{\"dom_id\":\"time_span\",\"payload\":\"" + std::string(buf) + "\",\"client_id\":\"\"}";

            for (auto it = ws_connections.begin(); it != ws_connections.end(); ) {
                struct mg_connection *conn = *it;
                if (conn && !conn->is_closing) {
                    mg_ws_send(conn, time_json.c_str(), time_json.length(), WEBSOCKET_OP_TEXT);
                    ++it;
                } else {
                    it = ws_connections.erase(it);
                }
            }
            last_time_update = now;
        }

        usleep(5000);  // Немного спим, чтобы не грузить CPU
    }

    mg_mgr_free(&mgr);
    return 0;
}