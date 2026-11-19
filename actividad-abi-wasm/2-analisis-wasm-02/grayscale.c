#include <stdint.h>
#include <stddef.h>

// DIP: Abstracción de filtro por píxel mediante puntero a función
typedef void (*PixelFilter)(uint8_t* r, uint8_t* g, uint8_t* b);

// SRP: Responsabilidad única de transformar un píxel RGB a escala de grises
static void grayscale_filter(uint8_t* r, uint8_t* g, uint8_t* b) {
    uint8_t gray = (uint8_t)((*r * 0.3f) + (*g * 0.59f) + (*b * 0.11f));
    *r = gray;
    *g = gray;
    *b = gray;
}

// OCP: Motor de iteración sobre buffer RGBA abierto a cualquier PixelFilter
static void process_image(uint8_t* pixels, int num_pixels, PixelFilter filter) {
    if (!pixels || num_pixels <= 0 || !filter) return;

    for (int i = 0; i < num_pixels * 4; i += 4) {
        // Ignora pixels[i + 3] para preservar el canal Alpha
        filter(&pixels[i], &pixels[i + 1], &pixels[i + 2]);
    }
}

// Punto de entrada exportado a WebAssembly
__attribute__((export_name("apply_grayscale")))
void apply_grayscale(uint8_t* pixels, int num_pixels) {
    process_image(pixels, num_pixels, grayscale_filter);
}