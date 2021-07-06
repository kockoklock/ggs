LIBS = -lpthread

main: s_main.cpp
	$(CXX) $? -o $@ $(LIBS)
