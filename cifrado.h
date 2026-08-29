#ifndef CIFRADO_H
#define CIFRADO_H

#include <sodium.h>
#include <string>

using namespace std;

string cifrarArchivo(const string &textoPlano, const unsigned char clave[32], const unsigned char salt[16], unsigned long long opslimit, size_t memlimit);
bool descifrarArchivo(const string &contenido, const unsigned char clave[32], string &textoPlano);
bool derivarClave(const string &password, const unsigned char salt[16], unsigned char clave[32], unsigned long long opslimit, size_t memlimit);
bool generarSalt(unsigned char salt[16]);
bool extraerSaltDeContenido(const string &contenido, unsigned char salt[16]);
bool extraerParametrosDeContenido(const string &contenido, unsigned long long &opslimit, size_t &memlimit);

#endif
