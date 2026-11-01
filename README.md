# Rescate USB

Diagnóstico **de solo lectura** para archivos de imagen de discos o memorias USB. No abre dispositivos físicos ni modifica la imagen.

## Funciones

- SHA-256 del archivo y comparación con hash esperado.
- MBR: firma, flags, límites y particiones solapadas.
- GPT: CRC de cabeceras y tabla, coherencia de las copias primaria/secundaria, límites y solapamientos.
- Salida JSON por stdout o a un archivo; modo `--debug` envía trazas a stderr.
- Builds y tests en Linux y Windows por GitHub Actions.

## Compilar

Requisitos: CMake 3.16+, C++17 y Python 3 para la prueba de integración de CLI.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/rescateusb imagen.img
./build/rescateusb imagen.img --json
./build/rescateusb imagen.img --json-out reporte.json --expected-sha256 HASH --debug
```

En Windows ejecuta `build/Release/rescateusb.exe`.

## Código de salida

- `0`: estructura válida y hash (si se indicó) coincidente.
- `1`: imagen sin tabla reconocida o con estructura no válida.
- `2`: argumentos o error de entrada/salida.
- `3`: el SHA-256 no coincide con el esperado.

## Seguridad y límites

Esta herramienta solo recibe rutas de archivos. No repara ni recupera datos, no verifica sistemas de archivos y no analiza dispositivos USB físicos. “Válido” significa únicamente que las comprobaciones de tabla soportadas pasaron; no demuestra que los datos estén íntegros o recuperables. Trabaja sobre una copia, conserva el original y no ejecutes herramientas de reparación sobre el medio original.
