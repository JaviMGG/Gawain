#include "gawain.h"
#include <iostream>
#include <random>
#include <algorithm>

using namespace std;

const char elementosTeclado[] = "1234567890qwertyuioplkjhgfdsazxcvbnmABCDEFGHIJKLMNOPQRSTUVWXYZ";

map<string, string> gestionador;

void añadirContraseñas(const string &app, const string &contraseña)
{
    gestionador[app] = contraseña;
}

string Vault::generarContraseña()
{
    static mt19937 generador(random_device{}());
    static uniform_int_distribution<int> distribucion(0, sizeof(elementosTeclado) - 2);
    string contraseña;
    for (int i = 0; i < 40; i++)
    {
        contraseña += elementosTeclado[distribucion(generador)];
    }
    return contraseña;
}

string Vault::getContraseñasToString(const string &app)
{
    return gestionador[app];
}

string Vault::getContraseñasToStringConDelimitador(const string &app)
{
    return "|" + gestionador[app];
}

void Vault::añadirContraseñas(const string &app, const string &contraseña)
{
    gestionador[app] = contraseña;
}

string Vault::listarContraseñas(const string &app)
{
    return gestionador[app];
}

string Vault::listarTodo()
{
    string todo;
    for (const auto &elemento : gestionador)
    {
        todo += " App: " + elemento.first + ", Contraseña: " + elemento.second + "\n";
    }
    return todo;
}

string Vault::buscarContraseña(const string &app)
{
    auto itApp = gestionador.find(app);
    if (itApp == gestionador.end())
    {
        return "";
    }
    return itApp->second;
}

bool Vault::eliminarContraseña(const string &app)
{
    auto itApp = gestionador.find(app);
    if (itApp == gestionador.end())
    {
        cout << "La app \"" << app << "\" no existe." << endl;
        return false;
    }

    gestionador.erase(itApp);
    return true;
}
