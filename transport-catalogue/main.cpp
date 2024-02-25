#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"

int main() {
    transport::Catalogue catalogue;
    JsonReader json_doc;
    json_doc.ReadInput(std::cin);
    if (auto base_requests = json_doc.GetRequests(JsonReader::KEY_BASE_REQUESTS)) {
        JsonReader::UploadData(catalogue, base_requests);
    }
    if (auto render_settings_requests = json_doc.GetRequests(JsonReader::KEY_RENDER_SETTINGS)) {
        auto render_settings = JsonReader::ParseRenderSettings(render_settings_requests);
        renderer::MapRenderer renderer(render_settings);
        RequestHandler handler(catalogue, renderer);
        if (auto stat_requests = json_doc.GetRequests(JsonReader::KEY_STAT_REQUESTS)) {
            JsonReader::GetStatInfo(handler, stat_requests);
        }
    }
}
