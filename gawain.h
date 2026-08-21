#ifndef GAWAIN_H
#define GAWAIN_H

#include <map>
#include <string>
#include <vector>

using namespace std;

extern map<string, vector<string>> gestionador;

string generarContraseña();
string getContraseñasToString(const vector<string> &contraseñas);
string getContraseñasToStringConDelimitador(const vector<string> &contraseñas);
void añadirContraseñas(const string &app, const string &contraseña);
string listarContraseñas(const vector<string> &contraseñas);
string listarTodo();
string buscarContraseña(const string &app);
bool eliminarContraseña(const string &app, const string &contraseña);
void guardar(const unsigned char clave[32], const unsigned char salt[16], unsigned long long opslimit, size_t memlimit);
bool cargar(const unsigned char clave[32]);

#endif
