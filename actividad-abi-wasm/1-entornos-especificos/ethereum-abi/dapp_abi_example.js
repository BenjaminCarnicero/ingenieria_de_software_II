import { ethers } from "ethers";

// Definición ABI legible del contrato inteligente
const abi = [
  "function recordFrame(uint256 pixels) external returns (uint256)",
  "event FrameRecorded(address indexed user, uint256 pixels, uint256 totalFrames)"
];

const iface = new ethers.Interface(abi);

// 1. Codificar calldata para invocar recordFrame(2073600) -> 1920x1080 píxeles
const totalPixels = 1920 * 1080;
const calldata = iface.encodeFunctionData("recordFrame", [totalPixels]);
console.log("Calldata codificado (hex):", calldata);

// 2. Decodificar los parámetros enviados a partir del calldata
const decodedArgs = iface.decodeFunctionData("recordFrame", calldata);
console.log("Argumentos decodificados:", decodedArgs[0].toString());

// 3. Decodificar un retorno simulado uint256 = 1
const rawReturn = "0x0000000000000000000000000000000000000000000000000000000000000001";
const decodedReturn = iface.decodeFunctionResult("recordFrame", rawReturn);
console.log("Total de frames procesados:", decodedReturn[0].toString());