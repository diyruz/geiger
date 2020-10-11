
-------------------------
Индикатор радиоактивности Zigbee

https://modkam.ru/?p=1591

https://github.com/diyruz/geiger
-------------------------


Сброс устройства для подключения к сети:
- Обесточить устройство (лучше всего кабель от источника питания отключать).
- Подать питание и дождаться, пока не загорится светодиод, тут же питание отключить на секунду и подать вновь.
- Повторить цикл подачи питания 5 раз. 
- Через некоторое время устройство начнет сопряжение с сетью ZigBee (на координаторе также нужно включить режим сопряжения).
  
-------------------------

Настройки:

# Тип трубки (0: СБМ-20/СТС-5/BOI-33; 1: СБМ-19/СТС-6; 3: все остальное):
mosquitto_pub -h mqtt_server -u mqtt_user -P mqtt_password -t 'zigbee2mqtt/0x00124b001ec7777e/1/set/sensors_type' -m '0' -d

# Кол-во трубок:
mosquitto_pub -h mqtt_server -u mqtt_user -P mqtt_password -t 'zigbee2mqtt/0x00124b001ec7777e/1/set/sensors_count' -m '2' -d
# Когда трубки две - показания меньше плавают, чем с одной. Но, мне кажется, показания несколько завышаются при этом.

# Число милирентген для аларма:
mosquitto_pub -h mqtt_server -u mqtt_user -P mqtt_password -t 'zigbee2mqtt/0x00124b001ec7777e/1/set/alert_threshold' -m '60' -d

# Мигание диодом на события радиации:
mosquitto_pub -h mqtt_server -u mqtt_user -P mqtt_password -t 'zigbee2mqtt/0x00124b001ec7777e/1/set/led_feedback' -m 'ON' -d

# НЕ ПОДДЕРДИВАЕТСЯ: Пищание на события радиации:

mosquitto_pub -h mqtt_server -u mqtt_user -P mqtt_password -t 'zigbee2mqtt/0x00124b001ec7777e/1/set/buzzer_feedback' -m 'ON' -d

# Удаление из сети:
mosquitto_pub -h mqtt_server -u mqtt_user -P mqtt_password -t 'zigbee2mqtt/bridge/config/remove' -m '0x00124b001ec7777e' -d

0x00124b001ec7777e - адрес или friendly name индикатора радиоактивности, поменять на свое значение

Не проверял:
Sensitivity: mosquitto_pub -t "zigbee2mqtt/FN/BUTTON_NUM/set/sensitivity" -m '100' This attribute will be used on reporting, pulsesCount * sensitivity. You can use this attribute to setup reporting in your prefered units radiationDosePerHour = pulsesCount * sensitivity

-------------------------

alert_threshold — выставляем уровень в мкР/ч при превышении которого сработает сигнализация

buzzer — встроенный зуммер (поддержка пока не реализована)

Led — включаем/выключаем светодиод отображающий регистрацию частиц

rph — регистрируемое значение в мкР/ч

rpm — регистрируемое значение частиц в минуту

sensitivity — чувствительность счетчика (используется если выбран пункт 3 в sensor_type)

sensor_type — выбор типа счетчика:

0) СБМ-20/СТС-5/BOI-33
1) СБМ-19/СТС-6
3) все остальное

sensors_count — количество установленных счетчиков


