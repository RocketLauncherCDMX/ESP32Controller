# ESP32Controller
<p align="center">
  <img src="https://github.com/user-attachments/assets/5c2477c7-2137-42b6-b296-f14735b3a114" alt="PS3 ESP32 Controller" width="1020"/>
</p>
<br>

Este es un Control tipo PS3, pero con una tarjeta basada en el popular **ESP32**, un microcontrolador de 32 bits muy potente, capaz de conectarse por **Bluetooth y Wifi**, además tiene 4MB de flash, lo que es ideal para cargarle programas grandes, como páginas web e incluso imágenes, ¿para qué? para poder hacer tus propias interfaces Web y controlar tus proyectos por medio de peticiones http, https o web sockets.

<br><br>

---

<p align="center">
  <img src="https://github.com/user-attachments/assets/9720bd97-8cd2-47d6-af0b-7525baae9c0e" alt="ESP32 Controller PCB" width="800"/>
</p>
<br>
<p align="center">
  <img src="https://github.com/user-attachments/assets/211f2dc1-2802-448b-9f14-bb10e4ce374c" alt="ESP32 Controller PCB" width="800"/>
</p>

---

Además tendrás la opción de ocuparlo de forma más tradicional, por medio de Bluetooth 4.1, con un consumo de corriente muy bajo y un alcance optimo (15 metros) o Wifi Direct, el cual es un protocolo de comunicación propietario de Expressif para conectar dispositivos punto a punto (sin la necesidad de un router) por medio de Wifi.

El ESP32 Controller tiene todas las funciones de un control de PS3 común:

- Cruceta de direcciones
- Botones equis, círculo, triángulo y cuadrado
- L1-L3 y R1-R3 digitales
- Select y Start
- Botón de encendido y apagado
- Joystick analógico izquierdo y derecho
- Vibrador suave y fuerte
- 4 LEDs de jugador, personalizables
- 2 LEDs RGB
- Batería de 1800mAh recargable
- Conector USB para carga y programación
- Wifi 2.4GHz
- Bluetooth 4.2
- Acelerómetro de 3 ejes
- 4 Colores a elegir: Azul, Rojo, Verde y Amarillo

---

Al sacarlo de su empaque, notarás que es exáctamente igual a un control de fábrica. Para encenderlo, basta con presionar brevemente el botón central **P3**. Si el control tiene batería, entonces un LED azul se encenderá justo debajo del botón, indicando que se el control se encuentra encendido. Si no enciende, entonces, muy posiblemente, necesite recargarse. Basta con conectarlo con su cable USB C a cualquier puerto capaz de suministrar 500mA y este comenzará a cargar. Un LED rojo indica que se encuentra en proceso de carga y un LED verde indica una carga completa.

🟦 Control encendido <br>
🟥 Cargando <br>
🟩 Carga completa

⚠️ **IMPORTANTE 1:** El botón **P3** no se puede programar, ya que su única función es encender/apagar el dispositivo.
⚠️ **IMPORTANTE 2:** Mientras el dispositivo se encuentre apagado, la conectividad USB será detectada normalmente, sin embargo, para programar y utilizar la consola Serial, es necesario encender el control.

---

## Primeros pasos

El control viene con un programa de ejemplo, el cual consiste en un enlace bluetooth. Lo primero para probarlo es descargar la app **nRF Connect**

<p align="center">
  <img src="https://github.com/user-attachments/assets/1be6b070-9348-46a1-b09f-a3cc2b4537c1" alt="nrf connet"/>
</p>

Puedes descargarlo para Android en este enlace:    
```bash 
https://play.google.com/store/search?q=nrf%20connect&c=apps&utm_source=latam_Med
```

Y para iOS en este otro:
```bash 
https://apps.apple.com/mx/app/nrf-connect-for-mobile/id1054362403
```

Al entrar en la app, veremos la lista de dispositivos disponibles. Si el control se encuentra encendido, verás uno de los 4 LEDs de jugador parpadeando, lo que significa que el bluetooth está activo pero sin conectar. Dependiendo del número de LED que se encuentre parpadeando, será el nombre del control que aparecerá en la app.

<p align="center">
  <br><br>
  <img src="https://github.com/user-attachments/assets/ee1ef0f2-a10e-42d5-9d33-4e02bddd33d4" alt="PS3 ESP32 Controller leds"/>
</p>

<br>
<br>

Entre tu y tus amigos podrán utilizar hasta 4 diferentes controles cada uno vinculado a un dispositivo, sin embargo, fácilmente se puede extender a 16 haciendo un par de cambios en el código. Para cambiar de Jugador (1 a 4) basta con presionar el botón **SELECT**. El control cambiará de nombre y se reiniciará, con lo que ahora aparecerá otro nombre y parpadeará otro LED.

<br>
<br>

<p align="center">
  <img src="https://github.com/user-attachments/assets/4c90e154-04e8-454a-9e03-a8717092e759" alt="nRF connect BLE devices" width="400"/>
</p>

<br>

Al presionar el botón **CONNECT** La pantalla mostrará el link con el control. Aparecerá un Atributo y un Acceso genericos, además de un Servicio desconocido. Este último corresponde a nuestro control.

<br>

<p align="center">
  <img src="https://github.com/user-attachments/assets/c3f2cbb9-2925-45bf-b53e-1d1bf7025767" alt="nRF connect BLE devices" width="400"/>
</p>

<br>

Al hacer click sobre este servicio, se expandirá, mostrando 3 características.

<br>

<p align="center">
  <img src="https://github.com/user-attachments/assets/57b2ecea-a7b6-46bd-bcd4-a0ce6a7b96fc" alt="BLE characteristics" width="400"/>
</p>

<br>

En el Bluetooth Low Energy (BLE) Un **servicio** corresponde a una conexión activa, y esta a su vez contrendrá una o varias **características** puede representar un valor, como por ejemplo, el nivel de batería, un sensor, un estado. En este ejemplo, tenemos 3 características. Cada característica contiene la siguiente información:

- **Servicio**
  - UUID ........................... 914b → Servicio primario
- **Características**
  - UUID ........................... 26a8  →  Botón Presionado
  - UUID ........................... 0e4f  →  Valor de los Joysticks analógicos
  - UUID ........................... 8520  →  Valor de la posición del control (acelerómetro)

<br>

Al hacer click en el ícono de las 3 flechas, se activarán las notificaciones (flechas tachadas), con lo que obtendremos la actualización de cada característica en tiempo real. Si no activamos las notificaciones (flechas normales), entonces podemos hacer click en el ícono de 1 flecha, el cual es para leer ese valor manualmente.

<br>

![nrfNotifications](https://github.com/user-attachments/assets/19eb1f8d-8486-4bea-b230-bafaf90f3d37)

<br>

![characteristicNotify](https://github.com/user-attachments/assets/6ed58703-0472-4a37-8265-85db173c113e)

<br>

Si activamos las 3 notificaciones, entonces estaremos recibiendo los valores del botón presionado, el valor de ambos Joysticks y el valor de la inclinación del control. Esta información está codificada de la siguiente forma:

```bash
UUID ........................... 26a8  → Muestra un caracter correspondiente al último botón que se presiono.
```
  Este ejemplo solamente puede registrar un solo botón presionado cada vez, pero se puede modificar para reportar todos los botones y si se presionaron o se dejaron de presionar. La lista de botones y su caracter se muestra a continuación:

| BOTON             | CARACTER     |
|-------------------|--------------|
| Arriba            | 'U'          |
| Abajo             | 'D'          |
| Izquierda         | 'L'          |
| Derecha           | 'R'          |
| Círculo           | 'C'          |
| Cuadrado          | 'Q'          |
| Triángulo         | 'T'          |
| Equis (X)         | 'X'          |
| L1                | '1'          |
| L2                | '2'          |
| L3                | '3'          |
| R1                | '4'          |
| R2                | '5'          |
| R3                | '6'          |
| Start             | 'S'          |

Como verás no tenemos acceso los botones **P3** ni **SELECT** ya que estos son parte de funciones propias del control.

<br>

---

```bash
UUID ........................... 0e4f  →  Muestra el valor de cada Joystick en hexadecimal, uno junto al otro
```
| VALOR            | IZQ-X | IZQ-Y | DER-X | DER_Y |
|------------------|-------|-------|-------|-------|
| B5-81-7F-36      | B5    | 81    | 7F    | 36    |

Cada que cualquiera de los 2 ejes de ambos Joysticks cambia de valor, se unen en un solo mensaje y se manda una notificación con los 4 valores, esto evita ralentizar el bus mandando todo el tiempo los valores.

---

```bash
UUID ........................... 8520  →  Muestra un par de valores correspondientes al ángulo de inclinación del control y su dirección
```

<br>

Esta característica es la responsable de capturar las notificaciones que envía el sensor deinclinación del control cuando la posición cambia.

Para simplicidad de la lectura del sensor de inclinación, el ángulo solo se reporta en niveles del 0 al 10, junto con la dirección a la que se está inclinando el control. Tenemos las siguientes direcciones de inclinación posibles:

| MOVIMIENTO | CARACTER |
|------------|----------| 
| Reposo     | ' '      |
| Adelante   | 'U'      |
| Atrás      | 'D'      |
| Izquierda  | 'L'      |
| Derecha    | 'R'      |

Cuando el Control está en reposo, reporta un espacio en blanco.
Cada caracter va acompañado de un valor (0 - 10), que indica qué tanto se ladeó el control en esa dirección, abarcando 90°, por ejemplo:

```bash
44-05
44 → 0x44 = ASCII 68 = D (ATRÄS)
05 → 0x05 = aproximadamente 50°
```

De esta forma puedes utilizar toda esta información para controlar cualquier dispositivo.

---

## Código de ejemplo para un dispositivo basado en ESP32

Llegados a este punto, te preguntarás cómo utilizar el control con alguno de tus proyectos, así que a continuación te dejamos el enlace al código de arduino de un ejemplo de aplicación del control, esto para una placa basada en un controlador ESP32, por ejemplo, puede utilizar la placa Tuxedo.

```bash

```
