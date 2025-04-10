#!/bin/env bash

if [ "$EUID" -ne 0 ]; then
  echo "Ejecute el script con sudo: sudo $0"
  exit 1
fi

CURRENT_USER=$(logname || echo "$SUDO_USER")
USER_HOME=$(eval echo ~$(logname))


# INSTALAR ARDUINO-CLI

echo "Instalando arduino-cli..."
sudo -u "$CURRENT_USER" mkdir -p $USER_HOME/.local/bin
sudo -u "$CURRENT_USER" curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sudo -u "$CURRENT_USER" BINDIR=$USER_HOME/.local/bin sh > /dev/null
sudo -u "$CURRENT_USER" $USER_HOME/.local/bin/arduino-cli core install arduino:avr > /dev/null


# CONFIGURAR PERMISOS PARA ACCEDER AL ARDUINO

echo "Configurando acceso al puerto serial..."

# Crear reglas
UDEV_RULES_FILE="/etc/udev/rules.d/99-arduino.rules"

echo "Creando reglas en $UDEV_RULES_FILE..."

cat > "$UDEV_RULES_FILE" << 'EOF'
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
usermod -a -G dialout "$CURRENT_USER"

# Recargar reglas
echo "Refrescando reglas..."
udevadm control --reload-rules
udevadm trigger

echo "Listo! Cierra y vuelve a iniciar sesión para completar"

exit 0