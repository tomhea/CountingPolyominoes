g++ -m64 -fPIC -c redelClient.cpp graphCreator.cpp backups.cpp

g++ -m64 -shared RedelClient.o graphCreator.o backups.o -o libRedelClient.dll

g++ -m64 -fPIC -c redelServer.cpp graphCreator.cpp backups.cpp

g++ -m64 -shared RedelServer.o graphCreator.o backups.o -o libRedelServer.dll
