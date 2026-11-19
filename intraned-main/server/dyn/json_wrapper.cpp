#include "../include/json.hpp"
#include <cstdlib>
#include <cstring>
#include <string>

using json = nlohmann::json;

static char* alloc_cstr(const std::string& s) {
    char* out = static_cast<char*>(std::malloc(s.size() + 1));
    if (out) {
        std::memcpy(out, s.c_str(), s.size() + 1);
    }
    return out;
}

extern "C" {

#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

// Devuelve: 1 = password ok, 0 = password mal, -1 = JSON invalido
EXPORT int dyn_check_login(const char* body, const char* expected_pass) {
    try {
        auto j = json::parse(body ? body : "");
        if (!j.contains("password") || !j["password"].is_string()) {
            return -1;
        }
        return (j["password"].get<std::string>() == expected_pass) ? 1 : 0;
    } catch (...) {
        return -1;
    }
}

// Formatea un string JSON (valida y devuelve dump()), o nullptr si es invalido
EXPORT char* dyn_json_dump(const char* raw_json) {
    try {
        auto j = json::parse(raw_json ? raw_json : "");
        return alloc_cstr(j.dump());
    } catch (...) {
        return nullptr;
    }
}

// Construye el JSON de metadata del archivo subido
EXPORT char* dyn_build_item_json(const char* titulo, const char* autor, const char* tema, const char* filename) {
    json nuevo = {
        {"titulo", titulo ? titulo : ""},
        {"autor", autor ? autor : ""},
        {"tema", tema ? tema : ""},
        {"file", filename ? filename : ""}
    };
    return alloc_cstr(nuevo.dump());
}

EXPORT void dyn_free_str(char* ptr) {
    std::free(ptr);
}

}