@echo off
REM Script de pruebas para GSEA
REM Windows CMD

echo ========================================
echo GSEA - Suite de Pruebas Automatizadas
echo ========================================
echo.

REM Crear directorio de pruebas si no existe
if not exist tests_auto mkdir tests_auto
cd tests_auto

echo [1/8] Creando archivos de prueba...
echo AAAAAABBBBBBCCCCCCDDDDDD > test_rle.txt
echo Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. > test_lzw.txt
echo Este es un archivo secreto con información confidencial. CONFIDENCIAL CONFIDENCIAL CONFIDENCIAL. > test_enc.txt
echo.

echo [2/8] Prueba 1: Compresion RLE
..\gsea.exe -c --comp-alg rle -i test_rle.txt -o test_rle.gsea
if %errorlevel% neq 0 (
    echo ERROR en compresion RLE
    goto :error
)
echo ✓ Compresion RLE exitosa

echo [3/8] Prueba 2: Descompresion RLE
..\gsea.exe -d --comp-alg rle -i test_rle.gsea -o test_rle_recovered.txt
if %errorlevel% neq 0 (
    echo ERROR en descompresion RLE
    goto :error
)
fc test_rle.txt test_rle_recovered.txt > nul
if %errorlevel% neq 0 (
    echo ERROR: Archivos no coinciden
    goto :error
)
echo ✓ Descompresion RLE exitosa - Integridad verificada

echo [4/8] Prueba 3: Compresion LZW
..\gsea.exe -c --comp-alg lzw -i test_lzw.txt -o test_lzw.gsea
if %errorlevel% neq 0 (
    echo ERROR en compresion LZW
    goto :error
)
echo ✓ Compresion LZW exitosa

echo [5/8] Prueba 4: Descompresion LZW
..\gsea.exe -d --comp-alg lzw -i test_lzw.gsea -o test_lzw_recovered.txt
if %errorlevel% neq 0 (
    echo ERROR en descompresion LZW
    goto :error
)
fc test_lzw.txt test_lzw_recovered.txt > nul
if %errorlevel% neq 0 (
    echo ERROR: Archivos no coinciden
    goto :error
)
echo ✓ Descompresion LZW exitosa - Integridad verificada

echo [6/8] Prueba 5: Compresion + Encriptacion (RLE + Vigenere)
..\gsea.exe -ce --comp-alg rle --enc-alg vigenere -i test_enc.txt -o test_rle_vig.gsea -k "ClaveSecreta123"
if %errorlevel% neq 0 (
    echo ERROR en compresion+encriptacion RLE+Vigenere
    goto :error
)
echo ✓ Compresion+Encriptacion RLE+Vigenere exitosa

echo [7/8] Prueba 6: Desencriptacion + Descompresion (RLE + Vigenere)
..\gsea.exe -du --comp-alg rle --enc-alg vigenere -i test_rle_vig.gsea -o test_rle_vig_recovered.txt -k "ClaveSecreta123"
if %errorlevel% neq 0 (
    echo ERROR en desencriptacion+descompresion RLE+Vigenere
    goto :error
)
fc test_enc.txt test_rle_vig_recovered.txt > nul
if %errorlevel% neq 0 (
    echo ERROR: Archivos no coinciden
    goto :error
)
echo ✓ Desencriptacion+Descompresion RLE+Vigenere exitosa - Integridad verificada

echo [8/8] Prueba 7: Compresion + Encriptacion (LZW + Feistel)
..\gsea.exe -ce --comp-alg lzw --enc-alg feistel -i test_enc.txt -o test_lzw_feistel.gsea -k "OtraClaveSegura"
if %errorlevel% neq 0 (
    echo ERROR en compresion+encriptacion LZW+Feistel
    goto :error
)
echo ✓ Compresion+Encriptacion LZW+Feistel exitosa

echo.
echo ========================================
echo TODAS LAS PRUEBAS PASARON EXITOSAMENTE
echo ========================================
echo.
echo Archivos generados en tests_auto/:
dir /b *.gsea
echo.
goto :end

:error
echo.
echo ========================================
echo ERROR: Una o mas pruebas fallaron
echo ========================================
echo.
cd ..
exit /b 1

:end
cd ..
echo Pruebas completadas. Los archivos de prueba estan en tests_auto/
exit /b 0
