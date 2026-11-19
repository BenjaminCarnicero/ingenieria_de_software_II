#!/usr/bin/env bash
set -e

GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}Iniciando compilación de la Isla de Conocimiento...${NC}"

mkdir -p build
mkdir -p uploads

UNAME_S=$(uname -s)
if [[ "$UNAME_S" == MINGW* || "$UNAME_S" == MSYS* || "$UNAME_S" == CYGWIN* ]]; then
    EXT="dll"
    WIN_FLAGS="-D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00"
    EXTRA_LIBS="-lws2_32 -lpthread"
    BIN_NAME="build/intraned.exe"
    LINK_FLAGS=""
else
    EXT="so"
    WIN_FLAGS=""
    EXTRA_LIBS="-lpthread"
    BIN_NAME="build/intraned"
    LINK_FLAGS="-ldl -pthread"
fi

g++ -O3 -fPIC -shared server/dyn/json_wrapper.cpp -Iserver/include -o build/libjson_dyn.$EXT
g++ -O3 -fPIC -shared $WIN_FLAGS server/dyn/http_wrapper.cpp -Iserver/include -o build/libhttp_dyn.$EXT $EXTRA_LIBS
g++ -O3 server/main.cpp $LINK_FLAGS -o "$BIN_NAME"

echo -e "${GREEN}=========================================${NC}"
echo -e "${GREEN} Compilación exitosa (.$EXT) en: $BIN_NAME ${NC}"
echo -e "${GREEN}=========================================${NC}"
echo -e "Para iniciar el servidor, ejecuta: $BIN_NAME"