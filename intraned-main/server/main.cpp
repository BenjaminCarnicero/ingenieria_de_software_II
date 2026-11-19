/**
- @file main.cpp
- @author Gabriel Nicolás González Ferreira
- @brief Servidor principal - Versión de alta compatibilidad (POSIX)
- @version 0.3
 */

#include "utils/fileHelper.cpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h> // Para mkdir y stat (POSIX)
#include <sys/types.h>

// 1. CARGA DINÁMICA (arriba, antes de main)
#ifdef _WIN32
  #include <windows.h>
  using LibHandle = HMODULE;
  inline LibHandle open_lib(const char* p) { return LoadLibraryA(p); }
  inline void* get_sym(LibHandle h, const char* s) { return reinterpret_cast<void*>(GetProcAddress(h, s)); }
  inline void close_lib(LibHandle h) { if (h) FreeLibrary(h); }
#else
  #include <dlfcn.h>
  using LibHandle = void*;
  inline LibHandle open_lib(const char* p) { return dlopen(p, RTLD_LAZY); }
  inline void* get_sym(LibHandle h, const char* s) { return dlsym(h, s); }
  inline void close_lib(LibHandle h) { if (h) dlclose(h); }
#endif

using CheckLoginFn = int (*)(const char*, const char*);
using JsonDumpFn = char* (*)(const char*);
using BuildItemJsonFn = char* (*)(const char*, const char*, const char*, const char*);
using FreeStrFn = void (*)(char*);

using LoginCb = int (*)(const char* body);
using GetContenidosCb = char* (*)();
using UploadCb = int (*)(
    const char* auth_header,
    int has_file,
    const char* filename,
    const char* content,
    size_t content_len,
    const char* titulo,
    const char* autor,
    const char* tema
);
using RunServerFn = int (*)(
    const char* host,
    int port,
    const char* public_dir,
    const char* upload_dir,
    LoginCb on_login,
    GetContenidosCb on_get_contenidos,
    UploadCb on_upload
);

static CheckLoginFn g_check_login = nullptr;
static JsonDumpFn g_json_dump = nullptr;
static BuildItemJsonFn g_build_item_json = nullptr;
static FreeStrFn g_free_str = nullptr;
static std::string g_upload_dir = "./server/uploads";
static const std::string ADMIN_TOKEN = "educacion_libre_2026";

void asegurar_directorio(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        #ifdef _WIN32
            mkdir(path.c_str());
        #else
            mkdir(path.c_str(), 0777);
        #endif
    }
}

// Callbacks usados por libhttp_dyn
static int handle_login(const char* body) {
    int r = g_check_login(body, "admin123");
    if (r == 1) return 200;
    if (r == 0) return 401;
    return 400;
}

static char* handle_get_contenidos() {
    std::ifstream file(g_upload_dir + "/metadata.json");
    if (!file.is_open()) return nullptr;

    std::ostringstream ss;
    ss << file.rdbuf();
    char* dumped = g_json_dump(ss.str().c_str());
    if (!dumped) return nullptr;

    std::string copy(dumped);
    g_free_str(dumped);

    char* out = static_cast<char*>(std::malloc(copy.size() + 1));
    if (out) std::memcpy(out, copy.c_str(), copy.size() + 1);
    return out;
}

static int handle_upload(
    const char* auth_header,
    int has_file,
    const char* filename,
    const char* content,
    size_t content_len,
    const char* titulo,
    const char* autor,
    const char* tema
) {
    if (!auth_header || ADMIN_TOKEN != auth_header) return 403;
    if (!has_file) return 400;

    std::string path = g_upload_dir + "/" + filename;
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs.is_open()) return 500;

    ofs.write(content, content_len);
    ofs.close();

    char* item_cstr = g_build_item_json(titulo, autor, tema, filename);
    if (!item_cstr) return 501;

    // Si FileHelper::registrarArchivo espera un nlohmann::json, pasale json::parse(item_cstr)
    bool ok = FileHelper::registrarArchivo(nlohmann::json::parse(item_cstr));
    g_free_str(item_cstr);

    return ok ? 200 : 501;
}

int main() {
    asegurar_directorio(g_upload_dir);

#ifdef _WIN32
    const char* json_path = "./build/libjson_dyn.dll";
    const char* http_path = "./build/libhttp_dyn.dll";
#else
    const char* json_path = "./build/libjson_dyn.so";
    const char* http_path = "./build/libhttp_dyn.so";
#endif

    // 2. Cargar bibliotecas dinámicas al inicio de main()
    LibHandle jlib = open_lib(json_path);
    LibHandle hlib = open_lib(http_path);
    if (!jlib || !hlib) {
        std::cerr << "Error cargando bibliotecas dinamicas (.dll / .so)\n";
        close_lib(jlib);
        close_lib(hlib);
        return 1;
    }

    g_check_login = reinterpret_cast<CheckLoginFn>(get_sym(jlib, "dyn_check_login"));
    g_json_dump = reinterpret_cast<JsonDumpFn>(get_sym(jlib, "dyn_json_dump"));
    g_build_item_json = reinterpret_cast<BuildItemJsonFn>(get_sym(jlib, "dyn_build_item_json"));
    g_free_str = reinterpret_cast<FreeStrFn>(get_sym(jlib, "dyn_free_str"));
    auto run_server = reinterpret_cast<RunServerFn>(get_sym(hlib, "dyn_run_server"));

    if (!g_check_login || !g_json_dump || !g_build_item_json || !g_free_str || !run_server) {
        std::cerr << "Error resolviendo simbolos en bibliotecas dinamicas\n";
        close_lib(jlib);
        close_lib(hlib);
        return 1;
    }

    // 3. Ejecutar servidor usando la biblioteca dinámica HTTP
    int rc = run_server(
        "0.0.0.0",
        8080,
        "./public",
        g_upload_dir.c_str(),
        handle_login,
        handle_get_contenidos,
        handle_upload
    );

    // 4. Liberar bibliotecas al cerrar
    close_lib(jlib);
    close_lib(hlib);
    return rc;
}