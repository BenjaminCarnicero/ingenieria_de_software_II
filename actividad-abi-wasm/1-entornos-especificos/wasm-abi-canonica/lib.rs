// Implementación del componente bajo el contrato WIT (ABI canónica)
pub struct PixelStats {
    pub width: u32,
    pub height: u32,
    pub processed_bytes: u64,
}

pub fn grayscale(mut rgba: Vec<u8>, width: u32, height: u32) -> (Vec<u8>, PixelStats) {
    for px in rgba.chunks_exact_mut(4) {
        let gray = (0.3 * px[0] as f32 + 0.59 * px[1] as f32 + 0.11 * px[2] as f32) as u8;
        px[0] = gray;
        px[1] = gray;
        px[2] = gray;
        // px[3] (Alpha) se conserva intacto
    }

    let stats = PixelStats {
        width,
        height,
        processed_bytes: rgba.len() as u64,
    };

    (rgba, stats)
}