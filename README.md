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

Entre tu y tus amigos podrán utilizar hasta 4 diferentes controles cada uno vinculado a un dispositivo, sin embargo, fácilmente se puede extender a 16 haciendo un par de cambios en el código. Para cambiar de Jugador (1 a 4) basta con presionar el botón **SELECT**. El control cambiará de nombre y se reiniciará, con lo que ahora aparecerá otro nombre y parpadeará otro LED.

<p align="center">
  <img src="https://github.com/user-attachments/assets/4c90e154-04e8-454a-9e03-a8717092e759" alt="nRF connect BLE devices" width="400"/>
</p>
