# Rescate USB

Diagnóstico de **archivos de imagen** de discos o memorias USB, estrictamente en modo de solo lectura. No abre dispositivos físicos, no modifica imágenes, no repara ni recupera datos.

## Funciones

- SHA-256 de la imagen y comparación opcional con un hash esperado.
- Validación MBR: firma, flags, límites y particiones solapadas.
- Validación GPT: CRC de cabeceras/tablas, coherencia entre copias primaria y secundaria, límites y solapamientos.
- Reporte en texto o JSON; el reporte JSON se puede guardar en un archivo.
- `--debug` escribe trazas a stderr.
- Tests unitarios y de integración, compilados en Linux y Windows por GitHub Actions.

## Estructura del código

- `src/main.cpp`: análisis de argumentos y presentación del resultado.
- `src/core.hpp`: tipos y contratos compartidos.
- `src/hash.cpp`: SHA-256.
- `src/partition_tables.cpp`: validación MBR/GPT y lectura segura por offsets.
- `src/image_analyzer.cpp`: carga de imagen y coordinación del análisis.
- `src/report.cpp`: serialización JSON.
- `src/self_tests.cpp`: fixtures y pruebas integradas del analizador.
- `tests/test_cli.py`: pruebas de integración de la interfaz CLI.

## Compilar y ejecutar

Requisitos: CMake 3.16+, compilador C++17 y Python 3 para la prueba de integración.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/rescateusb imagen.img
./build/rescateusb imagen.img --json
./build/rescateusb imagen.img --json-out reporte.json --expected-sha256 HASH --debug
```

En Windows, ejecuta `build/Release/rescateusb.exe`.

## Código de salida

- `0`: tabla validada y hash (si se indicó) coincidente.
- `1`: tabla no reconocida o estructura no válida.
- `2`: argumentos o error de entrada/salida.
- `3`: el SHA-256 no coincide con el esperado.

“Válido” significa solo que pasaron las comprobaciones de tabla implementadas; no garantiza la integridad del contenido ni la recuperación de datos. Trabaja sobre una copia y conserva el archivo original.
