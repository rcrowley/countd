from countd import client, message, commitlog
import select, socket

ADDRESS = "0.0.0.0" # TODO Configurable
PORT = 48879 # TODO Configurable

if "__main__" == __name__:

    cl = commitlog.CommitLog()

    # Listen
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((ADDRESS, PORT))
    s.listen(1)
    s.setblocking(0)

    epoll = select.epoll()
    epoll.register(s.fileno(), select.EPOLLIN)
    try:
        clients = {}
        while 1:
            events = epoll.poll(1)
            for fileno, event in events:

                if s.fileno() == fileno:
                    c = client.Write(*s.accept())
                    epoll.register(c.fileno(), select.EPOLLIN)
                    clients[c.fileno()] = c

                elif event & select.EPOLLIN:
                    try:
                        cl.write(clients[fileno].read())
                        clients[fileno].write(client.Write.WIN)
                    except OSError:
                        pass
                    except socket.error:
                        epoll.unregister(fileno)
                        del clients[fileno]
#                    epoll.modify(fileno, select.EPOLLOUT)

                elif event & select.EPOLLOUT:
                    pass

                elif event & select.EPOLLHUP:
                    epoll.unregister(fileno)
                    del clients[fileno]
    finally:
        epoll.unregister(s.fileno())
        epoll.close()
        s.close()
