const canvas = document.getElementById("canvas");
const ctx = canvas.getContext("2d");

// Dibujamos franjas de colores de prueba en el canvas
ctx.fillStyle = "rgb(255, 0, 0)";
ctx.fillRect(0, 0, 66, 100);
ctx.fillStyle = "rgb(0, 255, 0)";
ctx.fillRect(66, 0, 67, 100);
ctx.fillStyle = "rgb(0, 0, 255)";
ctx.fillRect(133, 0, 67, 100);

// Cargamos el módulo WASM puro sin glue code externo
const { instance } = await WebAssembly.instantiateStreaming(fetch("./grayscale.wasm"));
const { apply_grayscale, memory } = instance.exports;

document.getElementById("btn-gray").addEventListener("click", () => {
  const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
  const byteLength = imageData.data.length;
  const numPixels = byteLength / 4;

  // Aseguramos memoria suficiente en el módulo WASM
  const neededPages = Math.ceil((1024 + byteLength) / 65536);
  const currentPages = memory.buffer.byteLength / 65536;
  if (neededPages > currentPages) {
    memory.grow(neededPages - currentPages);
  }

  // Copiamos el buffer RGBA a la memoria lineal de WASM y ejecutamos
  const ptr = 1024;
  const wasmView = new Uint8Array(memory.buffer, ptr, byteLength);
  wasmView.set(imageData.data);

  apply_grayscale(ptr, numPixels);

  // Volcamos el resultado procesado de vuelta al canvas
  imageData.data.set(wasmView);
  ctx.putImageData(imageData, 0, 0);
});