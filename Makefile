bin:
	mkdir bin
verde: bin
	g++ -Wall -Wextra src/nodo_verde/Nodo_verde.cc src/nodo_verde/agente_naranja/Agente_naranja.cc \
	src/nodo_verde/agente_verde/Agente_verde.cc src/nodo_verde/agente_azul/Agente_azul.cc \
	src/nodo_verde/agente_rosado/Agente_rosado.cc src/nodo_verde/agente_red/Agente_red.cc \
	src/common/cola/Cola.cc -pthread -o bin/NodoVerde
run:
	bin/./NodoVerde bin/fifoPipeAgenteNaranja bin/fifoPipeNodoNaranja tests/Pandemic_Plan_Remastered.nodo.1.csv \
	tests/Pandemic_Plan_Remastered.vecinos.1.csv bin/fifoPipeAgenteAzul bin/fifoPipeNodoAzul 8100 7500 7600 127.0.0.1
server:
	cd src/nodo_azul/servidor; \
	python3 httpd.py 8000 127.0.0.1:8100
clean:
	rm bin/fifoPipeAgenteAzul bin/fifoPipeNodoAzul bin/fifoPipeAgenteNaranja bin/fifoPipeNodoNaranja
ayuda:
	bin/./NodoVerde --help
