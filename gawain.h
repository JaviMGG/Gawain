#ifndef GAWAIN_H
#define GAWAIN_H

#include <map>
#include <string>
#include <vector>
#include <cstdlib>

using namespace std;

inline string rutaBaseDatos()
{
    const char *home = getenv("HOME");
    return string(home ? home : ".") + "/.gawain/archivo.txt";
}

extern map<string, string> gestionador;

void añadirContraseñas(const string &app, const string &contraseña);
bool guardar(const unsigned char clave[32], const unsigned char salt[16], unsigned long long opslimit, size_t memlimit);
bool cargar(const unsigned char clave[32]);

class Vault {
public:
    string generarContraseña();
    string getContraseñasToString(const string &app);
    string getContraseñasToStringConDelimitador(const string &app);
    void añadirContraseñas(const string &app, const string &contraseña);
    string listarContraseñas(const string &app);
    string listarTodo();
    string buscarContraseña(const string &app);
    bool eliminarContraseña(const string &app);
};

#endif
