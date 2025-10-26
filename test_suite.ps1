# Script de pruebas para GSEA
# PowerShell

Write-Host "========================================"
Write-Host "GSEA - Suite de Pruebas Automatizadas"
Write-Host "========================================"
Write-Host ""

# Crear directorio de pruebas si no existe
if (-not (Test-Path tests_auto)) {
    New-Item -ItemType Directory -Path tests_auto | Out-Null
}
Set-Location tests_auto

Write-Host "[1/9] Creando archivos de prueba..."
[System.IO.File]::WriteAllText("test_rle.txt", "AAAAAABBBBBBCCCCCCDDDDDD", [System.Text.Encoding]::ASCII)
[System.IO.File]::WriteAllText("test_lzw.txt", "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.", [System.Text.Encoding]::ASCII)
[System.IO.File]::WriteAllText("test_enc.txt", "Este es un archivo secreto con información confidencial. CONFIDENCIAL CONFIDENCIAL CONFIDENCIAL.", [System.Text.Encoding]::ASCII)
Write-Host ""

$allPassed = $true

# Test 1: Compresión RLE
Write-Host "[2/9] Prueba 1: Compresión RLE"
& ..\gsea.exe -c --comp-alg rle -i test_rle.txt -o test_rle.gsea
if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ ERROR en compresión RLE" -ForegroundColor Red
    $allPassed = $false
} else {
    Write-Host "✓ Compresión RLE exitosa" -ForegroundColor Green
}

# Test 2: Descompresión RLE
Write-Host "[3/9] Prueba 2: Descompresión RLE"
& ..\gsea.exe -d --comp-alg rle -i test_rle.gsea -o test_rle_recovered.txt
if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ ERROR en descompresión RLE" -ForegroundColor Red
    $allPassed = $false
} elseif ((Compare-Object (Get-Content test_rle.txt) (Get-Content test_rle_recovered.txt))) {
    Write-Host "❌ ERROR: Archivos no coinciden" -ForegroundColor Red
    $allPassed = $false
} else {
    Write-Host "✓ Descompresión RLE exitosa - Integridad verificada" -ForegroundColor Green
}

# Test 3: Compresión LZW
Write-Host "[4/9] Prueba 3: Compresión LZW"
& ..\gsea.exe -c --comp-alg lzw -i test_lzw.txt -o test_lzw.gsea
if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ ERROR en compresión LZW" -ForegroundColor Red
    $allPassed = $false
} else {
    Write-Host "✓ Compresión LZW exitosa" -ForegroundColor Green
}

# Test 4: Descompresión LZW
Write-Host "[5/9] Prueba 4: Descompresión LZW"
& ..\gsea.exe -d --comp-alg lzw -i test_lzw.gsea -o test_lzw_recovered.txt
if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ ERROR en descompresión LZW" -ForegroundColor Red
    $allPassed = $false
} elseif ((Compare-Object (Get-Content test_lzw.txt) (Get-Content test_lzw_recovered.txt))) {
    Write-Host "❌ ERROR: Archivos no coinciden después de LZW" -ForegroundColor Red
    $allPassed = $false
} else {
    Write-Host "✓ Descompresión LZW exitosa - Integridad verificada" -ForegroundColor Green
}

# Test 5: Compresión + Encriptación (RLE + Vigenere)
Write-Host "[6/9] Prueba 5: Compresión + Encriptación (RLE + Vigenere)"
& ..\gsea.exe -ce --comp-alg rle --enc-alg vigenere -i test_enc.txt -o test_rle_vig.gsea -k "ClaveSecreta123"
if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ ERROR en compresión+encriptación RLE+Vigenere" -ForegroundColor Red
    $allPassed = $false
} else {
    Write-Host "✓ Compresión+Encriptación RLE+Vigenere exitosa" -ForegroundColor Green
}

# Test 6: Desencriptación + Descompresión (RLE + Vigenere)
Write-Host "[7/9] Prueba 6: Desencriptación + Descompresión (RLE + Vigenere)"
& ..\gsea.exe -du --comp-alg rle --enc-alg vigenere -i test_rle_vig.gsea -o test_rle_vig_recovered.txt -k "ClaveSecreta123"
if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ ERROR en desencriptación+descompresión RLE+Vigenere" -ForegroundColor Red
    $allPassed = $false
} elseif ((Compare-Object (Get-Content test_enc.txt) (Get-Content test_rle_vig_recovered.txt))) {
    Write-Host "❌ ERROR: Archivos no coinciden después de RLE+Vigenere" -ForegroundColor Red
    $allPassed = $false
} else {
    Write-Host "✓ Desencriptación+Descompresión RLE+Vigenere exitosa - Integridad verificada" -ForegroundColor Green
}

# Test 7: Solo Encriptación Vigenere
Write-Host "[8/9] Prueba 7: Solo Encriptación Vigenere"
& ..\gsea.exe -e --enc-alg vigenere -i test_enc.txt -o test_vig_only.gsea -k "Clave789"
if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ ERROR en encriptación Vigenere" -ForegroundColor Red
    $allPassed = $false
} else {
    Write-Host "✓ Encriptación Vigenere exitosa" -ForegroundColor Green
}

# Test 8: Solo Desencriptación Vigenere
Write-Host "[9/9] Prueba 8: Solo Desencriptación Vigenere"
& ..\gsea.exe -u --enc-alg vigenere -i test_vig_only.gsea -o test_vig_only_recovered.txt -k "Clave789"
if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ ERROR en desencriptación Vigenere" -ForegroundColor Red
    $allPassed = $false
} elseif ((Compare-Object (Get-Content test_enc.txt) (Get-Content test_vig_only_recovered.txt))) {
    Write-Host "❌ ERROR: Archivos no coinciden después de solo Vigenere" -ForegroundColor Red
    $allPassed = $false
} else {
    Write-Host "✓ Desencriptación Vigenere exitosa - Integridad verificada" -ForegroundColor Green
}

Write-Host ""
if ($allPassed) {
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "✓ TODAS LAS PRUEBAS PASARON EXITOSAMENTE" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Archivos generados en tests_auto/:"
    Get-ChildItem *.gsea | Format-Table Name, Length -AutoSize
    Set-Location ..
    exit 0
} else {
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "❌ UNA O MÁS PRUEBAS FALLARON" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
    Set-Location ..
    exit 1
}
