# Rescate USB

Herramienta CLI inicial para inspeccionar **archivos de imagen** de memorias/discos USB en modo de solo lectura. No escribe en la imagen ni opera directamente sobre dispositivos físicos.

## Funciones

- Calcula SHA-256 completo de la imagen.
- Detecta MBR y valida firma, indicadores de arranque y límites de particiones.
- Detecta GPT y comprueba CRC32 del encabezado y de la tabla de particiones, geometría y límites.
- Compara opcionalmente con un SHA-256 esperado.

## Compilar

Requiere CMake 3.16+ y un compilador C++17.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/rescateusb --self-test
./build/rescateusb imagen-usb.img
./build/rescateusb imagen-usb.img --expected-sha256 HASH_DE_64_CARACTERES
```

Windows: el ejecutable compilado con CMake se encuentra en `build/Release/rescateusb.exe`.

## Códigos de salida

- `0`: tabla reconocida y válida (o self-test aprobado).
- `1`: imagen leída, pero estructura ausente o inválida.
- `2`: uso incorrecto o error de lectura.
- `3`: el SHA-256 no coincide con el esperado.

## GitHub Releases

Al publicar una etiqueta `v*`, GitHub Actions compila y prueba binarios Linux x86_64 y Windows x86_64 y los adjunta a la Release. La ejecución manual compila y prueba los binarios, sin publicar.

## Alcance y seguridad

MVP inicial: procesa archivos de imagen existentes; no incluye interfaz gráfica, recuperación de datos, reparación/escritura, detección de hardware ni validación de sistemas de archivos. Verifica la imagen fuente y conserva una copia antes de realizar cualquier operación de recuperación. Un resultado estructural válido no demuestra que todo el contenido del medio sea íntegro.
