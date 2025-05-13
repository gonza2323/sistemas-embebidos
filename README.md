# Sistemas Embebidos

## Requerimientos

- Python

- (Opcional) VSCode y Extensión Wokwi Simulator para VSCode [Link](https://marketplace.visualstudio.com/items?itemName=wokwi.wokwi-vscode). Esto es si queremos simular el Arduino y no utilizar una placa Arduino real.
    
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

Una vez instalados los requerimientos, tenemos dos opciones para ejecutar la aplicación de algún TP. Conectando una placa Arduino a la PC, o utilizando el simulador Wokwi para Arduino.


### A) Usando la placa Arduino

Ejecutar el script de setup, que instala arduino-cli, la configuración de la placa, y configura los permisos correctamente para que el usuario pueda acceder al puerto serial. Este paso solo es necesario una vez.

```bash
./arduino_setup.sh
```

Para ejecutar un TP, primero debe conectarse la placa por USB. Luego podemos ejecutar el script `./upload.sh <nombre-tp>` para compilar y cargar el programa del tp correspondiente a la placa. `<nombre-tp>` corresponde al nombre del directorio de algún TP. Por ejemplo, para el trabajo práctico 2, sería `tp2`, y no `tp2/`.

Para que funcione, el programa del Arduino debe estar en `<nombre-tp>/arduino/arduino.ino`.

Si hay problemas de permisos para acceder al Arduino, es porque nunca se ejecutó el script `./arduino_setup.sh`.


### B) Usando el simulador de Arduino

El simulador no arranca si no está compilado el programa de Arduino. Para ello, desde el directorio del proyecto, ejecutamos el script `./compile <nombre-tp>` para compilar el programa de Arduino del tp correspondiente. Por ejemplo:

```bash
./compile.sh tp3
```

Para que funcione, el programa del Arduino debe estar en `<nombre-tp>/arduino/arduino.ino`.

Ahora sí, podemos arrancar el simulador en VSCode apretando `F1` o `Shift`+`Ctrl`+`P` y buscando la opción "Wokwi: Select Config File", seleccionando la configuración del tp que queramos, y luego buscar y ejecutar la opción "Wokwi: Start Simulator".

**La simulación debe estar visible para que no se detenga, conviene colocarla como una nueva pestaña a un costado en el VSCode.**


### Arrancar el servidor web

Para arrancar el servidor, tenemos varias opciones:

```
# con un script de utilidad (utiliza flask)
./start_server <nombre-tp>
```

```
# como módulo de python
python3 -m <nombre-tp>.website.main
```

```
# con flask
flask --app <nombre-tp>.website.main run
```

Una vez andando el servidor, se puede acceder a la página en http://localhost:5000

Se puede setear la variable de entorno `DEBUG_SERIAL=1` para obtener logs de debugging de la conexión serial. Si ejecutamos con flask o el script, el flag `--debug` también activará el debugging de la conexión serial.

El servidor funciona tanto con la placa Arduino como con el simulador Wokwi. Si se pierde la conexión, intenta restablecerla con cualquiera de los dos que se encuentre disponible. Si ambos están disponibles, dará prioridad a la placa.

## Prueba

En la página web para el TP1, deberían funcionar los controles de los LEDs tanto en el simulador como en la placa Arduino. Lo mismo para la lectura de la simulación.

![screenshot del entorno](./img/screenshot.png)
