from practicum import findDevices
from peri import PeriBoard
from time import sleep

devs = findDevices()

if len(devs) == 0:
    print "*** No MCU board found."
    exit(1)

b = PeriBoard(devs[0])
print "*** MCU board found"
print "*** Device manufacturer: %s" % b.getVendorName()
print "*** Device name: %s" % b.getDeviceName()

count = 0
while True:
    sleep(0.5)
    #b.setLedValue(count)
    #sw = b.getSwitch()
    light = b.getLight()
    temp = b.getTemp()
    humi = b.getHumi()
    stage = ""
    if light > 2*1023/3:
         stage = "high"
    elif (light > 1023/3) and (light < 2*1023/3) :
         stage = "median"
    else:
         stage = "low"
    print "LEDs set to %d | Light value: %s | Temperature: %d^C | Humidity: %d" % (
            count, light, temp, humi)

    count = (count + 1) % 8
    

