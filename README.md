# EspUdpTrigger
UDP trigger for ESP8266 / ESP-WROOM-02.

## Dependency

* Arduino core for ESP8266 WiFi chip
[https://github.com/esp8266/arduino-esp8266fs-plugin](https://github.com/esp8266/arduino-esp8266fs-plugin)
* Arduino ESP8266 filesystem uploader
[https://github.com/esp8266/Arduino](https://github.com/esp8266/Arduino)

## Usage

1. Download to your sketch folder for Arduino.
2. Open the EspUdpTrigger.ino.
3. Upload sketch data directory using [arduino-esp8266fs-plugin](https://github.com/esp8266/arduino-esp8266fs-plugin).
4. Upload the sketch file via serial from UART mode.
5. Switch to the Flash mode.
6. Reboot the ESP.
7. Join the ESP AP.(SSID="esp", PASSWORD="12345678")
8. Access to `[http://192.168.4.1/](http://192.168.4.1/)`
9. Set to the [Device Name], [SSID], [PASS] from the Settings panel.
10. Access to `http://[Device Name].local/` or IP Address after Reboot.

## API

# | URL | Name
--- | --- | ---
1 | [http://Controller_01.local/](http://Controller_01.local/) | Root Menu. This page.
2 | [http://Controller_01.local/pin](http://Controller_01.local/pin]) | Set GPIO pin value.<br>ex) `http://Controller_01.local/pin?no=5&value=255`
3 | [http://Controller_01.local/config](http://Controller_01.local/config) | Set device name, SSID, PASS.<br>ex) `http://Controller_01.local/config?name=servo&ssid=wifi_router&pass=12345678&triggers_0_pin=12&triggers_0_ip=192.168.0.100&triggers_0_port=20000`
4 | [http://Controller_01.local/init](http://Controller_01.local/init) | Restore the default settings.

## Screenshot

![Screenshot01](https://github.com/TsubokuLab/EspUdpTrigger/blob/master/screenshot/esp_udp_01.png)
![Screenshot02](https://github.com/TsubokuLab/EspUdpTrigger/blob/master/screenshot/esp_udp_02.png)
![Screenshot03](https://github.com/TsubokuLab/EspUdpTrigger/blob/master/screenshot/esp_udp_03.png)
![Screenshot04](https://github.com/TsubokuLab/EspUdpTrigger/blob/master/screenshot/esp_udp_04.png)
![Screenshot04](https://github.com/TsubokuLab/EspUdpTrigger/blob/master/screenshot/esp_udp_05.png)

## Author

Teruaki Tsubokura [http://teruaki-tsubokura.com/](http://teruaki-tsubokura.com/)

[@kohack_v](https://twitter.com/kohack_v)

## License

[MIT](https://mit-license.org/)