# Rescate USB

Herramienta de consola para diagnosticar **archivos de imagen** de memorias/discos USB en modo de solo lectura. No modifica la imagen ni escribe en dispositivos físicos.

## Funciones

- Calcula SHA-256 de la imagen y permite compararlo con un hash esperado.
- Detecta MBR: firma, indicadores de arranque y límites de particiones.
- Detecta GPT y valida CRC32 de encabezado y tabla, copia primaria/secundaria, consistencia de metadatos, límites y solapamientos de particiones.
- Compila para Linux y Windows con C++17 y CMake.

## Compilar y usar

Requiere CMake 3.16+ y un compilador C++17.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/rescateusb --self-test
./build/rescateusb imagen-usb.img
./build/rescateusb imagen-usb.img --expected-sha256 HASH_DE_64_CARACTERES
```

En Windows ejecuta `build/Release/rescateusb.exe`.

## Códigos de salida

- `0`: tabla reconocida como válida o self-test aprobado.
- `1`: estructura no reconocida o inválida.
- `2`: error de argumentos o lectura.
- `3`: SHA-256 distinto del esperado.

## GitHub Releases

Cada etiqueta `v*` activa el workflow: compila y prueba Linux x86_64 y Windows x86_64, y publica ambos ejecutables en una GitHub Release.

## Alcance y precauciones

Procesa archivos de imagen; no detecta medios físicos, no repara ni recupera archivos y no valida sistemas de archivos. Una tabla de particiones válida no garantiza la integridad de todos los datos. Conserva una copia de seguridad antes de intentar cualquier recuperación.
