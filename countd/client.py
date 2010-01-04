from countd import message
import sys

class Read(object):
    pass

class Write(object):

    # Response codes
    WIN = 0
    FAIL = 1

    def __init__(self, connection, address):
        self.connection = connection
        self.connection.setblocking(0)
        self.address = address
        sys.stderr.write("[client] new connection from %s:%d\n" % self.address)

    def __del__(self):
        self.connection.close()
        #sys.stderr.write("[client] connection from %s:%d closed\n" % self.address)

    def fileno(self):
        return self.connection.fileno()

    def read(self):
        return message.Write(self.connection.recv(message.Write.LENGTH))

    def write(self, code):
        # TODO Response code
        return 3 == self.connection.send("WIN")

if "__main__" == __name__:
    Write(None, None)
