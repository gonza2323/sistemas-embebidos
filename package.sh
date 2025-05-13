#!/usr/bin/env bash

PACKAGED_DIR="packaged"

rm -rf "$PACKAGED_DIR"
mkdir -p "$PACKAGED_DIR"

# directorios de los TPs
TPS=("tp1" "tp2" "tp3" "tp5" "integrador")

# configurar permisos
chmod u+x ./*.sh
chmod u+x package-files/*.sh

# copiar archivos
for TP in "${TPS[@]}"; do

    # TPs
    if [ -d "$TP" ]; then
        cp -r "$TP" "$PACKAGED_DIR/"
    else
        echo "Error: No se encontró el directorio de $TP"
    fi

    # common/
    if [ -d "common" ]; then
        cp -r common/ "$PACKAGED_DIR/$TP/website/"
    else
        echo "Error: No se encontró el directorio de 'common/'"
        exit 1
    fi

    # package-files/
    if [ -d "package-files" ]; then
        cp -r package-files/* "$PACKAGED_DIR/$TP/"
    else
        echo "Error: No se encontró el directorio package-files/"
        exit 1
    fi

    # arduino.setup.sh
    if [ -f "arduino_setup.sh" ]; then
        cp "arduino_setup.sh" "$PACKAGED_DIR/$TP/"
    else
        echo "Error: No se encontró 'arduino_setup.sh'"
        exit 1
    fi

    # requirements.txt
    if [ -f "requirements.txt" ]; then
        cp "requirements.txt" "$PACKAGED_DIR/$TP/"
    else
        echo "Error: No se encontró 'requirements.txt'"
        exit 1
    fi
    
    # Remover directorios 'build/' y '__pycache__/'
    find "$PACKAGED_DIR/$TP" -type d \( -name "build" -o -name "__pycache__" \) -exec rm -rf {} +

    # Armar zip
    cd "$PACKAGED_DIR"
    zip -r "$TP.zip" "$TP" > /dev/null
    cd - > /dev/null
done



echo "Completado. Se empaquetaron los TPs en '$PACKAGED_DIR'"
