#!/bin/env bash


# Verificar que no se ejecutó con sudo
if [ "$EUID" -eq 0 ]; then
  echo "No ejecute el script como superusuario"
  exit 1
fi


# INSTALAR ARDUINO-CLI

if command -v arduino-cli &> /dev/null; then
    echo "arduino-cli ya está instalado"
else
  INSTALL_DIR="$HOME/.local/bin"

  echo "Instalando arduino-cli..."
  mkdir -p "$HOME/.local/bin"
  sudo -u "$USER" curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=$INSTALL_DIR sh > /dev/null

  # Agregar ~/.local/bin al PATH si no lo está
  CONFIG_FILE="$HOME/.bash_profile"
  [ ! -f "$CONFIG_FILE" ] && CONFIG_FILE="$HOME/.profile"

  if ! echo "$PATH" | grep -q "$HOME/.local/bin"; then
      echo "" >> "$CONFIG_FILE"
      echo "# Added by installer" >> "$CONFIG_FILE"
      echo 'if [[ ":$PATH:" != *":$HOME/.local/bin:"* ]]; then' >> "$CONFIG_FILE"
      echo '    PATH="$HOME/.local/bin:$PATH"' >> "$CONFIG_FILE"
      echo 'fi' >> "$CONFIG_FILE"
      echo "Se agregó arduino-cli ($INSTALL_DIR) al PATH"

      export PATH="$HOME/.local/bin:$PATH"
      echo -e "\e[1;33mEjecute source $CONFIG_FILE o cierre la consola para refrescar el PATH!!!\e[0m"
  fi

  # Instalar configuración de la placa
  arduino-cli core install arduino:avr > /dev/null    
fi


# CONFIGURAR PERMISOS PARA ACCEDER AL ARDUINO

# Crear reglas

echo "Configurando acceso al puerto serial..."
echo "Se requieren permisos de superusuario para configurar el puerto serial"

UDEV_RULES_FILE="/etc/udev/rules.d/99-arduino.rules"
sudo echo "Creando reglas en $UDEV_RULES_FILE..."

sudo tee "$UDEV_RULES_FILE" > /dev/null << 'EOF'
# Arduino Uno and Mega 2560
SUBSYSTEM=="tty", ATTRS{idVendor}=="2341", GROUP="dialout", MODE="0666", SYMLINK+="arduino"
# Arduino Uno WiFi Rev2
SUBSYSTEM=="tty", ATTRS{idVendor}=="03eb", GROUP="dialout", MODE="0666", SYMLINK+="arduino"
# Arduino SA
SUBSYSTEM=="tty", ATTRS{idVendor}=="2a03", GROUP="dialout", MODE="0666", SYMLINK+="arduino"
# Adafruit boards
SUBSYSTEM=="tty", ATTRS{idVendor}=="239a", GROUP="dialout", MODE="0666", SYMLINK+="arduino"
EOF

echo "Añadiendo usuario al grupo dialout..."
sudo usermod -a -G dialout "$USER"

# Recargar reglas
echo "Refrescando reglas..."
sudo udevadm control --reload-rules
sudo udevadm trigger

echo "Listo! Tal vez deba volver a iniciar sesión para completar"

exit 0
