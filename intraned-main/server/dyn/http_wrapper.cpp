#ifdef _WIN32
  #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0601
  #endif
#endif

#include "../include/httplib.h"
#include <string>

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

extern "C" {

#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

EXPORT int dyn_run_server(
    const char* host,
    int port,
    const char* public_dir,
    const char* upload_dir,
    LoginCb on_login,
    GetContenidosCb on_get_contenidos,
    UploadCb on_upload
) {
    httplib::Server svr;
    svr.set_mount_point("/", public_dir);
    svr.set_mount_point("/recursos", upload_dir);

    svr.Post("/api/login", [on_login](const httplib::Request& req, httplib::Response& res) {
        int status = on_login(req.body.c_str());
        if (status == 200) {
            res.set_content("{\"token\": \"educacion_libre_2026\"}", "application/json");
        } else if (status == 401) {
            res.status = 401;
            res.set_content("{\"error\": \"No autorizado\"}", "application/json");
        } else {
            res.status = 400;
            res.set_content("{\"error\": \"JSON invalido\"}", "application/json");
        }
    });

    svr.Get("/api/contenidos", [on_get_contenidos](const httplib::Request&, httplib::Response& res) {
        char* json_str = on_get_contenidos();
        if (json_str) {
            res.set_content(json_str, "application/json");
        } else {
            res.set_content("{\"contenidos\": []}", "application/json");
        }
    });

    svr.Post("/api/upload", [on_upload](const httplib::Request& req, httplib::Response& res) {
        std::string auth = req.get_header_value("Authorization");
        int has_file = req.form.has_file("archivo") ? 1 : 0;

        std::string filename, content, titulo, autor, tema;
        if (has_file) {
            const auto& file_part = req.form.get_file("archivo");
            filename = file_part.filename;
            content = file_part.content;
            titulo = req.form.get_field("titulo");
            autor = req.form.get_field("autor");
            tema = req.form.get_field("tema");
        }

        int code = on_upload(
            auth.c_str(),
            has_file,
            filename.c_str(),
            content.data(),
            content.size(),
            titulo.c_str(),
            autor.c_str(),
            tema.c_str()
        );

        res.status = code;
        if (code == 200) {
            res.set_content("{\"status\": \"success\"}", "application/json");
        } else if (code == 403) {
            res.set_content("{\"error\": \"Token invalido\"}", "application/json");
        } else if (code == 400) {
            res.set_content("{\"error\": \"Falta el archivo\"}", "application/json");
        } else if (code == 501) {
            res.set_content("{\"error\": \"Error en metadata\"}", "application/json");
        } else {
            res.set_content("{\"error\": \"Error de E/S en disco\"}", "application/json");
        }
    });

    std::cout << "Servidor iniciado en http://localhost:" << port << " (Modo dinamico)" << std::endl;
    return svr.listen(host, port) ? 0 : 1;
}

}