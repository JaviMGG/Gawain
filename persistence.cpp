#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include "gawain.h"
#include "cifrado.h"

using namespace std;

void guardar(const unsigned char clave[32], const unsigned char salt[16], unsigned long long opslimit, size_t memlimit)
{
    string textoPlano;
    for (auto &&elemento : gestionador)
    {
        textoPlano += elemento.first + elemento.second + "\n";
    }

    string textoCifrado = cifrarArchivo(textoPlano, clave, salt, opslimit, memlimit);

    ofstream salida("archivo.txt");
    salida << textoCifrado << endl;
    chmod("archivo.txt", S_IRUSR | S_IWUSR);
}

bool cargar(const unsigned char clave[32])
{
    ifstream entrada("archivo.txt");
    if (!entrada)
    {
        return true;    // no hay archivo: nada que cargar
    }

    string contenido;
    getline(entrada, contenido);

    string textoPlano;
    if (!descifrarArchivo(contenido, clave, textoPlano))
    {
        return false;
    }

    stringstream flujoTexto(textoPlano);
    string linea;
    while (getline(flujoTexto, linea))
    {
        if (linea.empty())
        {
            continue;
        }
        stringstream flujo(linea);
        string app;
        getline(flujo, app, '|');
        string contraseña;
        while (getline(flujo, contraseña, '|'))
        {
            añadirContraseñas(app, contraseña);
        }
    }
    return true;
}
