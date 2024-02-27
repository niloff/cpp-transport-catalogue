#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"
//#include <fstream>

int main() {
    transport::Catalogue catalogue;
    JsonReader json_doc;
    // разбираем данные из потока
//    std::ifstream base_input("s10_final_opentest_2.json");
    json_doc.ReadInput(std::cin);
    // инициализируем средства визуализации в соответствии с настройками
    const auto& render_settings = json_doc.GetRenderSettings();
    renderer::MapRenderer renderer(render_settings);
    // инициализируем обработчик запросов
    RequestHandler handler(renderer);
    // загружаем данные в каталог
    json_doc.UploadData(handler);
    // обрабатываем запросы
//    std::ofstream of("output.json");
    json_doc.PrintResponses(handler, std::cout);
}
