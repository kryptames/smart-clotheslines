from practicum import McuBoard


####################################
class PeriBoard(McuBoard):

    ################################
    def setLed(self, led_no, led_val):
        '''
        Set status of LED led_no on peripheral board to led_val
        '''
        self.usbWrite(request=0,index=led_no,value=led_val)

    ################################
    def setLedValue(self, value):
        '''
        Display value's 3 LSBs on peripheral board's LEDs
        '''
        self.usbWrite(request=1,value=value)

    
    ################################
    def getSwitch(self):
        '''
        Return a boolean value indicating whether the switch on the peripheral
        board is currently pressed
        '''
        result = self.usbRead(request=2,length=1)
        return result[0] == 1

    ################################
    def getLight(self):
        '''
        Return the current reading of light sensor on peripheral board
        '''
        result = self.usbRead(request=3,length=2)
        return result[0] + 256*result[1]

    def getTemp(self):
        result = self.usbRead(request=4,length=1)
        return result[0]

    def getHumi(self):
        result = self.usbRead(request=5,length=1)
        return result[0]


