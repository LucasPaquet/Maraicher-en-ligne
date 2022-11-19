.SILENT:

all:	Serveur CreationBD Publicite Caddie AccesBD Gerant Client

Gerant:	./object/maingerant.o ./object/windowgerant.o ./object/moc_windowgerant.o
	echo Creation de Gerant
	g++ -Wno-unused-parameter -o Gerant ./object/maingerant.o ./object/windowgerant.o ./object/moc_windowgerant.o /usr/lib64/libQt5Widgets.so /usr/lib64/libQt5Gui.so /usr/lib64/libQt5Core.so /usr/lib64/libGL.so -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl
./object/maingerant.o:	maingerant.cpp
	echo Creation de maingerant.o
	g++ -Wno-unused-parameter -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../Administrateur -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o ./object/maingerant.o maingerant.cpp
./object/windowgerant.o:	windowgerant.cpp
	echo Creation de windowgerant.o
	g++ -Wno-unused-parameter -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../Administrateur -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -I/usr/include/mysql -m64 -L/usr/lib64/mysql -o ./object/windowgerant.o windowgerant.cpp
./object/moc_windowgerant.o:	moc_windowgerant.cpp
	echo Creation de moc_windowgerant.o
	g++ -Wno-unused-parameter -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../Administrateur -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o ./object/moc_windowgerant.o moc_windowgerant.cpp

Client:	./object/mainclient.o ./object/windowclient.o ./object/moc_windowclient.o
	echo Creation de Client
	g++ -Wno-unused-parameter -o Client ./object/mainclient.o ./object/windowclient.o ./object/moc_windowclient.o  /usr/lib64/libQt5Widgets.so /usr/lib64/libQt5Gui.so /usr/lib64/libQt5Core.so /usr/lib64/libGL.so -lpthread
./object/mainclient.o:	mainclient.cpp
	echo Creation de mainclient.o
	g++ -Wno-unused-parameter -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../UNIX_DOSSIER_FINAL -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o ./object/mainclient.o mainclient.cpp
./object/windowclient.o:	windowclient.cpp
	echo Creation de windowclient.o
	g++ -Wno-unused-parameter -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../UNIX_DOSSIER_FINAL -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o ./object/windowclient.o windowclient.cpp
./object/moc_windowclient.o:	moc_windowclient.cpp
	echo Creation de moc_windowclient.o
	g++ -Wno-unused-parameter -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../UNIX_DOSSIER_FINAL -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o ./object/moc_windowclient.o moc_windowclient.cpp

Serveur:	Serveur.cpp
	echo Creation de Serveur
	g++ Serveur.cpp -o Serveur -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl
CreationBD:	CreationBD.cpp
	echo Creation de CreationBD
	g++ -o CreationBD CreationBD.cpp -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl
Publicite:	Publicite.cpp
	echo Creation de Publicite
	g++ -o Publicite Publicite.cpp
Caddie:	Caddie.cpp
	echo Creation de Caddie
	g++ -o Caddie Caddie.cpp -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl
AccesBD:	AccesBD.cpp
	echo Creation de AccesBD
	g++ -o AccesBD AccesBD.cpp -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl

clean:
	echo Supression de tout les .o
	rm -f ./object/*.o
clobber:
	echo Supression de tout les executables
	rm -f ./Publicite
	rm -f ./Serveur
	rm -f ./Clientµ
	rm -f ./Gerant
	rm -f ./CreationBD
	rm -f ./Client
	rm -f ./Caddie
	rm -f ./AccesBD