#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"
#include <fstream>

int main() {
    JsonReader json_doc;
    // разбираем данные из потока
//    std::ifstream base_input("e4_input.json");
    json_doc.ReadInput(std::cin);
    // инициализируем средства визуализации в соответствии с настройками
    const auto& render_settings = json_doc.GetRenderSettings();
    renderer::MapRenderer renderer(render_settings);
    // инициализируем средства маршрутизации в соответствии с настройками
    const auto& routing_settings = json_doc.GetRoutingSettings();
    transport::Router router(routing_settings);
    // инициализируем обработчик запросов
    RequestHandler handler(renderer, router);
    // загружаем данные в каталог
    json_doc.UploadData(handler);
    // обрабатываем запросы
//    std::ofstream of("out.json");
    json_doc.PrintResponses(handler, std::cout);
}
