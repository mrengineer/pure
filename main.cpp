#include "mongoose.h"
#include <string>
#include <ctime>
#include <vector>
#include <algorithm>
#include <map>
#include <unistd.h>  // Для usleep

std::vector<struct mg_connection *> sse_connections;  // Список SSE соединений
std::map<std::string, struct mg_connection *> client_map;  // Карта client_id -> соединение

extern std::string handle_command(const std::string& json_str);

static void event_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_match(hm->uri, mg_str("/sse"), NULL)) {
            // Ручная реализация SSE: отправляем headers
            mg_http_reply(c, 200, "Content-Type: text/event-stream\r\nCache-Control: no-cache\r\nAccess-Control-Allow-Origin: *\r\n\r\n", "");
            // Извлекаем client_id из query
            char client_id_buf[64] = {0};
            mg_http_get_var(&hm->query, "client_id", client_id_buf, sizeof(client_id_buf));
            if (client_id_buf[0]) {
                client_map[client_id_buf] = c;
            }
            sse_connections.push_back(c);  // Добавляем в список
        } else if (mg_match(hm->uri, mg_str("/command"), NULL) && mg_strcmp(hm->method, mg_str("POST")) == 0) {
            // Обработка команды от клиента
            std::string response = handle_command(std::string(hm->body.buf, hm->body.len));
            if (!response.empty()) {
                mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", response.c_str());
            } else {
                mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK");
            }
        } else {
            struct mg_http_serve_opts opts = {.root_dir = "static/"};
            mg_http_serve_dir(c, hm, &opts);
        }
    } else if (ev == MG_EV_CLOSE) {
        // Удаляем из списка при закрытии
        sse_connections.erase(std::remove(sse_connections.begin(), sse_connections.end(), c), sse_connections.end());
        // Удаляем из карты client_id
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
    mg_log_set(MG_LL_NONE);  // Отключаем все логи
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:8000", event_handler, NULL);
    printf("Сервер запущен на порту 8000\n");
    for (;;) {
        mg_mgr_poll(&mgr, 5000);  // Таймаут 5 секунд
        // Отправить время всем SSE соединениям, если они есть
        if (!sse_connections.empty()) {
            time_t now = time(NULL);
            char buf[64];
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
            for (auto it = sse_connections.begin(); it != sse_connections.end();) {
                struct mg_connection *c = *it;
                if (c->is_closing == 0) {
                    mg_printf(c, "data: %s\n\n", buf);
                    ++it;
                } else {
                    it = sse_connections.erase(it);  // Удаляем закрытые
                }
            }
        }
        usleep(50000);  // Задержка 0.1 сек для снижения нагрузки
    }
    mg_mgr_free(&mgr);
    return 0;
}