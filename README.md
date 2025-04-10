# Sistemas Embebidos

## Requerimientos

- Python

- VSCode y Extensión Wokwi Simulator para VSCode [Link](https://marketplace.visualstudio.com/items?itemName=wokwi.wokwi-vscode). Esto es si queremos simular el Arduino y no utilizar la placa real.
    
    Después de instalarla, hay que apretar `F1` en VSCode y buscar la opción "Wokwi: Request New License". Se abrirá un navegador y hacemos click en "Get Your License". Pide iniciar sesión.

- Arduino CLI y config de la placa

- Python venv

    Revisar si está instalado con `python3 -m venv`. Si no, instalarlo con pip o apt.


## Cómo usar el entorno de desarrollo

Clonar el repositorio.

```bash
clone git@github.com:gonza2323/sistemas-embebidos
```

Ir al directorio del repositorio, crear un entorno virtual y activarlo.

```bash
cd sistemas-embebidos
python3 -m venv .venv
source .venv/bin/activate
```

Si funcionó, ahora debería aparecer `(.venv)` en el prompt.

Dentro del entorno, instalar los requerimientos:

```bash
pip install -r requirements.txt
```

Ejecutar el script de setup, que instala arduino-cli, la configuración de la placa, y configura los permisos correctamente para que el usuario pueda acceder al puerto serial.

```bash
./setup.sh
```

Una vez instalados los requerimientos, tenemos dos opciones para ejecutar la aplicación. Utilizar el simulador de Arduino, o conectar la placa a la PC.

### A) Usando el simulador de Arduino

El simulador no arranca si no está compilado el programa de Arduino. Para ello, desde el directorio del proyecto, ejecutamos el script `./compile <directorio-tp>` para compilar el programa de Arduino del tp correspondiente. Por ejemplo:

```bash
./compile.sh tp3
```

Para que funcione, el programa del Arduino debe estar en `<directorio-tp>/arduino/arduino.ino`.

Ahora sí, podemos arrancar el simulador en VSCode apretando `F1` o `Shift`+`Ctrl`+`P` y buscando la opción "Wokwi: Select Config File", seleccionando la configuración del tp que queramos, y luego buscar y ejecutar la opción "Wokwi: Start Simulator".

**La simulación debe estar visible para que no se detenga, conviene colocarla como una nueva pestaña a un costado en el VSCode.**

### B) Usando la placa Arduino

Primero debe conectarse la placa por USB. Luego podemos ejecutar el script `./upload.sh <directorio-tp>` para compilar y cargar el programa del tp correspondiente a la placa.

Si hay problemas de permisos, es porque no se ejecutó nunca el script de setup `./setup.sh`. Alternativamente, ejecutar `sudo chmod 777 /dev/ttyACM0`

## Arrancar el servidor web

El servidor web no arranca si no está ejecutándose el simulador, o alternativamente, se encuentra conectada la placa Arduino.

Dentro del directorio del proyecto, ejecutamos el script `./start_server.sh <directorio-tp>`. Si estamos utilizando el simulador de Arduino, debemos agregar el flag `--debug`, de lo contrario no arrancará el servidor.

Una vez andando el servidor, se puede acceder a la página en http://localhost:5000

## Prueba

En la página web para el TP1, deberían funcionar los controles de los LEDs tanto en el simulador como en la placa Arduino. Lo mismo para la lectura de la simulación.

![screenshot del entorno](./img/screenshot.png)
