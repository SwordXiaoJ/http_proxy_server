CFLAGS= -std=c++11 -pedantic -Wall -Werror -pthread
proxy: proxy.o client.o handler.o request.o response.o server.o  cache.o handlerHelper.o
	g++ $(CFLAGS) -o proxy proxy.o client.o handler.o request.o response.o server.o  cache.o handlerHelper.o
%.o: %.cpp
	g++ $(CFLAGS) -c $<
.PHONY: clean
clean:
	rm -f proxy *.o