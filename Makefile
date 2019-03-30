all: sample2D

sample2D: Sample_GL3_2D.cpp glad.c
	g++ -o sample2D Sample_GL3_2D.cpp glad.c -lGL -lglfw -ldl -I "./include" -L"/usr/lib" ./libIrrKlang.so -pthread

clean:
	rm sample2D
