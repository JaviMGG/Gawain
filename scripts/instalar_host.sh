#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
HOST_BIN="$PROJECT_DIR/build/gawain-host"
MANIFEST_DIR="$HOME/.mozilla/native-messaging-hosts"
MANIFEST="$MANIFEST_DIR/gawain-host.json"

if [ ! -f "$HOST_BIN" ]; then
    echo "Error: $HOST_BIN no existe. Compila primero:"
    echo "  cmake -B $PROJECT_DIR/build -S $PROJECT_DIR"
    echo "  cmake --build $PROJECT_DIR/build"
    exit 1
fi

mkdir -p "$MANIFEST_DIR"

cat > "$MANIFEST" <<EOF
{
    "name": "gawain-host",
    "description": "Puente Firefox <-> Gawain",
    "path": "$HOST_BIN",
    "type": "stdio",
    "allowed_extensions": ["gawain@javimmgg"]
}
EOF

chmod +x "$HOST_BIN"
echo "Host instalado en $MANIFEST"
echo "Binario: $HOST_BIN"
