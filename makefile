LIBRARIES = -lcoap-1 
INCLUDE = -I /usr/local/include/coap/
CXXFLAGS = -D_GNU_SOURCE -DWITH_POSIX -g

all: server client client2 iclient pclient

server:server.c
	gcc server.c $(INCLUDE) $(LIBRARIES) $(CXXFLAGS) -g -o  server

client:client.c
	gcc client.c $(INCLUDE) $(LIBRARIES) $(CXXFLAGS) -g -o  client

client2:client2.c
	gcc client2.c $(INCLUDE) $(LIBRARIES) $(CXXFLAGS) -g -o  client2

iclient:client.c
	gcc iclient.c $(INCLUDE) $(LIBRARIES) $(CXXFLAGS) -g -o  iclient

pclient:pclient.c
	gcc pclient.c $(INCLUDE) $(LIBRARIES) $(CXXFLAGS) -g -o  pclient

clean:
	rm -f client client2 iclient pclient server
