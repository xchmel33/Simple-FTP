CC=g++
CFLAGS= -g -Wall

CLIENT = ipk-simpleftp-client
SERVER = ipk-simpleftp-server
DB = database
COM = communication

client: cleanC $(CLIENT)
server: cleanS $(SERVER)

$(CLIENT): $(CLIENT).cpp
	$(CC) $(CFLAGS) -o $(CLIENT) $(CLIENT).cpp $(COM).cpp

$(SERVER): $(SERVER).cpp
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER).cpp $(DB).cpp $(COM).cpp

cleanC:
	$(RM) $(CLIENT)
	$(RM) $(COM)

cleanS:
	$(RM) $(SERVER)
	$(RM) $(DB)
	$(RM) $(COM)


