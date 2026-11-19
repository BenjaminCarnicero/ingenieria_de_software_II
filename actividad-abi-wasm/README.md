# Implementación de ABIs en Entornos Específicos y Procesamiento de Imágenes en WebAssembly (`ANALISIS_WASM-02`)

Este proyecto reúne ejemplos prácticos sobre el uso de **ABIs (Application Binary Interfaces)** en WebAssembly y Ethereum, junto con la implementación de un filtro de escala de grises en tiempo real sobre buffers RGBA en WebAssembly aplicando principios **SOLID**.

---

## 1. Uso de ABI en Entornos Específicos

### A) WebAssembly Component Model (ABI Canónica)
- **Ubicación:** `1-entornos-especificos/wasm-abi-canonica/`
- **Objetivo:** Demostrar cómo el *Component Model* de WebAssembly estandariza la representación y transferencia de tipos de datos complejos (listas, registros, tuplas) entre distintos lenguajes sin depender del layout interno de memoria ni de punteros crudos.
- **Implementación:** Se define la interfaz en **WIT** (`filter.wit`) y su implementación en **Rust** (`lib.rs`).

### B) Ethereum y Smart Contracts (Contract ABI)
- **Ubicación:** `1-entornos-especificos/ethereum-abi/`
- **Objetivo:** Mostrar cómo una aplicación descentralizada (dApp) utiliza la ABI de un contrato inteligente en Solidity (`GrayscaleRegistry.sol`) para codificar los parámetros de llamada (`calldata`) y decodificar los valores devueltos y eventos (`dapp_abi_example.js`).

---

## 2. Implementación de `ANALISIS_WASM-02`: Escala de Grises RGBA en WebAssembly

- **Ubicación:** `2-analisis-wasm-02/`
- **Escenario:** Procesamiento de buffers unidimensionales de píxeles en formato **RGBA** (4 bytes por píxel: Rojo, Verde, Azul y Transparencia/Alpha) evitando cuellos de botella por recolección de basura en JavaScript.

### Arquitectura y Principios SOLID
1. **Responsabilidad Única (SRP):** `grayscale_filter` se encarga únicamente de calcular la luminancia estándar ($0.3 \cdot R + 0.59 \cdot G + 0.11 \cdot B$) y transformar un píxel.
2. **Abierto/Cerrado (OCP) e Inversión de Dependencias (DIP):** El motor de recorrido `process_image` recibe un puntero a función (`PixelFilter`), lo que permite incorporar nuevos filtros (sepia, inversión, umbral) sin modificar el bucle de iteración sobre el buffer RGBA ni alterar el canal Alpha.
3. **Eficiencia en el borde JS/WASM:** Siguiendo las conclusiones de `ANALISIS_WASM-02`, el núcleo compilado en C mantiene una interfaz plana y minimalista sobre `WebAssembly.Memory`, evitando *glue code* externo y *overhead* de clases virtuales en bucles críticos por píxel.

---

## 3. Compilación y Ejecución (`2-analisis-wasm-02`)

### Compilar `grayscale.c` a `grayscale.wasm`
Con **Clang / LLVM**:
```bash
clang --target=wasm32 -O3 -nostdlib -Wl,--no-entry -Wl,--export=apply_grayscale -Wl,--export-memory grayscale.c -o grayscale.wasm