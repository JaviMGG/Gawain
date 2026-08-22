#ifndef GAWAIN_H
#define GAWAIN_H

#include <map>
#include <string>
#include <vector>

using namespace std;

extern map<string, string> gestionador;
extern unsigned char salt[16];
extern unsigned char clave[32];
extern unsigned long long opslimit;
extern size_t memlimit;

string generarContraseña();
string getContraseñasToString(const string &app);
string getContraseñasToStringConDelimitador(const string &app);
void añadirContraseñas(const string &app, const string &contraseña);
string listarContraseñas(const string &app);
string listarTodo();
string buscarContraseña(const string &app);
bool eliminarContraseña(const string &app, const string &contraseña);
void guardar(const unsigned char clave[32], const unsigned char salt[16], unsigned long long opslimit, size_t memlimit);
bool cargar(const unsigned char clave[32]);

#endif
