import socket
import ubnode

class Moodlamp(ubnode.UBNode):
    """ Class for moodlamps connected to the uberbus. """
    def __init__(self, address, udp = False):
        """
        Create a moodlamp object.

        Before issuing any commands to the moodlamp connect() has to be called.
        
        Parameters:
        address  --  Hostname or IP address of the node.
        udp      --  True for connection less UDP protocol.
        """
        ubnode.UBNode.__init__(self,address,2323,udp)

    def setcolor(self, r, g, b):
        """
        Set the color of the lamp.

        Parameters:
        r,g,b -- The RGB value of the color(0-255 each)

        Returns True if the command was successfull.

        """
        cmd = "C%c%c%c"%(r,g,b)
        return self.sendCommand(cmd)

    def getcolor(self):
        """
        Get the color of the lamp.

        Return a list with the RGB values of the lamp or False if the command failed.

        """

        cmd = "g"
        if self.sendCommand(cmd):
            return self.readResponse(self.socket)
        else:
            return False

    def fade(self, r, g, b, speed):
        h = int(speed)>>8
        l = int(speed)&0xFF
        cmd = "F%c%c%c%c%c"%(r,g,b,h,l);
        return self.sendCommand(cmd)

    def timedfade(self, r, g, b, time, allchannelsequal=True):
        """ Fade to a color in a fixed time.

        This command fades the moodlamp to the specified RGB value within time seconds.

        Parameters:
        rgb              --  The RGB value.
        time             --  The time in seconds.
        allchannelsequal --  If True all channels will reach their goal at the same time.
                             If False all channels fade with the same speed with each cannel
                             reaching its goal independently.(Default: True)
        
        Returns True if the command was sucessfull.
        
        """
        time = time * 1000
        h = int(time)>>8
        l = int(time)&0xFF
        if allchannelsequal:
            fadecmd = 'T'
        else:
            fadecmd = 'M'
        cmd = "%c%c%c%c%c%c"%(fadecmd,r,g,b,h,l);
        return self.sendCommand(cmd)

    def setBrightness(self, brightness):
        cmd = "D%c"%brightness;
        return self.sendCommand(cmd)

    def getVersion(self):
        cmd = 'V'
        if self.sendCommand(cmd):
            return self.readResponse(self.socket)
        else:
            return False
