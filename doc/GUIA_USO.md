# GSEA - Gene Sequence Encryption & Archival Tool

## Guía de Compilación y Uso

### Compilación

En Windows con GCC:
```cmd
gcc -Wall -Wextra -O2 -std=c17 -pthread -o gsea.exe src/main.c src/cli.c src/fs.c src/worker.c src/pipeline.c src/rle.c src/lzw.c src/vigenere.c src/feistel.c src/crc32.c src/header.c src/util.c -pthread
```

En Linux/macOS con Make:
```bash
make
```

### Uso Básico

```bash
./gsea [operaciones] --comp-alg {rle|lzw} --enc-alg {vigenere|feistel} -i ENTRADA -o SALIDA [-k CLAVE] [-t MAX_HILOS]
```

#### Operaciones:
- `-c` : Comprimir
- `-d` : Descomprimir
- `-e` : Encriptar
- `-u` : Desencriptar

Las operaciones se pueden combinar, por ejemplo:
- `-ce` : Comprimir y luego encriptar
- `-du` : Desencriptar y luego descomprimir

#### Opciones:
- `--comp-alg {rle|lzw}` : Algoritmo de compresión
- `--enc-alg {vigenere|feistel}` : Algoritmo de encriptación  
- `-i RUTA` : Archivo o directorio de entrada
- `-o RUTA` : Archivo o directorio de salida
- `-k CLAVE` : Clave secreta (requerida para `-e` y `-u`)
- `-t NUM` : Número máximo de hilos concurrentes (por defecto: 8)

### Ejemplos

#### 1. Comprimir un archivo con RLE
```bash
./gsea -c --comp-alg rle -i datos.txt -o datos.gsea
```

#### 2. Comprimir y encriptar con LZW + Feistel
```bash
./gsea -ce --comp-alg lzw --enc-alg feistel -i secreto.txt -o secreto.gsea -k "MiClaveSegura123"
```

#### 3. Desencriptar y descomprimir
```bash
./gsea -du --comp-alg lzw --enc-alg feistel -i secreto.gsea -o secreto_recuperado.txt -k "MiClaveSegura123"
```

#### 4. Procesar un directorio completo
```bash
./gsea -ce --comp-alg rle --enc-alg vigenere -i ./datos/ -o ./archivados/ -k "clave" -t 16
```

### Formato de Archivo .gsea

Los archivos procesados tienen un header propietario:

```
┌─────────────────────────┐
│ Magic: 'GSEA' (4 bytes) │
│ Version: 1 (2 bytes)    │
│ Flags: (1 byte)         │  bit0: comprimido? bit1: encriptado?
│ Comp Algorithm (1 byte) │
│ Enc Algorithm (1 byte)  │
│ Reserved (1 byte)       │
│ Original Size (8 bytes) │
│ CRC32 (4 bytes)         │
├─────────────────────────┤
│ Datos procesados...     │
└─────────────────────────┘
```

### Algoritmos

#### RLE (Run-Length Encoding)
- **Ventaja**: Muy rápido, excelente para datos con secuencias repetitivas
- **Desventaja**: Puede expandir datos sin patrones repetitivos
- **Uso ideal**: Secuencias genéticas, imágenes simples, logs

#### LZW (Lempel-Ziv-Welch)
- **Ventaja**: Buen ratio de compresión, adaptativo
- **Desventaja**: Más lento que RLE, mayor uso de memoria
- **Uso ideal**: Texto general, datos con patrones variables

#### Vigenère
- **Ventaja**: Muy rápido (XOR), bajo overhead
- **Desventaja**: Menos seguro que cifrados modernos
- **Uso ideal**: Ofuscación rápida, protección básica

#### Feistel (16 rondas)
- **Ventaja**: Más seguro, basado en estructura tipo-DES
- **Desventaja**: Más lento que Vigenère
- **Uso ideal**: Datos sensibles, cumplimiento normativo

### Pruebas

```bash
# Crear archivo de prueba
echo "AAAAAABBBBBBCCCCCC Este es un texto de prueba." > test.txt

# Comprimir y encriptar
./gsea -ce --comp-alg rle --enc-alg vigenere -i test.txt -o test.gsea -k "clave123"

# Verificar tamaño
ls -lh test.*

# Descomprimir y desencriptar
./gsea -du --comp-alg rle --enc-alg vigenere -i test.gsea -o test_recovered.txt -k "clave123"

# Verificar integridad
diff test.txt test_recovered.txt
```

### Arquitectura

```
┌──────────────┐
│   main.c     │  ← Punto de entrada, parseo CLI
└──────┬───────┘
       │
┌──────▼───────┐
│   worker.c   │  ← Gestión de concurrencia (pthreads)
└──────┬───────┘
       │
┌──────▼────────┐
│  pipeline.c   │  ← Orquestación de operaciones
└───┬───────┬───┘
    │       │
┌───▼───┐ ┌─▼───────┐
│ rle.c │ │ vigenere│
│ lzw.c │ │ feistel │
└───────┘ └─────────┘
    │           │
┌───▼───────────▼───┐
│      fs.c         │  ← Llamadas al sistema
└───────────────────┘
```

### Notas Importantes

1. **Integridad**: Todos los archivos `.gsea` incluyen CRC32 para detectar corrupción
2. **Orden de operaciones**: Siempre comprimir antes de encriptar (`-ce`), desencriptar antes de descomprimir (`-du`)
3. **Clave**: La misma clave debe usarse para encriptar y desencriptar
4. **Concurrencia**: Al procesar directorios, cada archivo se procesa en paralelo
5. **Errores**: Si la descompresión/desencriptación falla, verifica la clave y el algoritmo usado

### Limitaciones Conocidas

- LZW limitado a códigos de 12 bits (4096 entradas)
- Vigenère es vulnerable a análisis de frecuencia (solo para uso educativo/protección básica)
- El padding en Feistel es simple (no PKCS#7)
- Sem semáforos POSIX pueden no estar disponibles en todas las plataformas Windows

### Solución de Problemas

**Error: "Falta clave (-k)"**
- Las operaciones `-e` y `-u` requieren especificar una clave

**Error al procesar archivo**
- Verifica que uses el mismo algoritmo para comprimir/descomprimir
- Verifica que la clave sea correcta para operaciones de encriptación

**El archivo comprimido es más grande**
- Normal con RLE en datos sin patrones. Usa LZW para mejor compresión general

**Errores de compilación en Windows**
- Asegúrate de tener MinGW o similar con soporte pthread
- Usa `-pthread` tanto en CFLAGS como en LDFLAGS
