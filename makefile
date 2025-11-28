CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

server: main.cpp  ./timer/lst_timer.cpp ./http/http_conn.cpp ./log/log.cpp ./CGImysql/sql_connection_pool.cpp  webserver.cpp config.cpp
	$(CXX) -o server  $^ $(CXXFLAGS) -lpthread -lmysqlclient

crow_server: crow_server.cpp ./CGImysql/sql_connection_pool.cpp ./log/log.cpp
	$(CXX) -o crow_server -g $^ $(CXXFLAGS) -I./third_party -lmysqlclient -lpthread -Wno-return-type


clean:
	rm  -r server crow_server