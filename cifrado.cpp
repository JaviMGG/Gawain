#include "cifrado.h"
#include <sstream>
#include <vector>
#include <iostream>

using namespace std;

static const char TABLA_HEX[] = "0123456789abcdefhijklmnoprqstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

static string bytesAHex(const unsigned char *bytes, size_t longitud)
{
    string resultado;
    resultado.reserve(longitud * 2);
    for (size_t i = 0; i < longitud; i++)
    {
        resultado += TABLA_HEX[bytes[i] >> 4];
        resultado += TABLA_HEX[bytes[i] & 0x0f];
    }
    return resultado;
}

static int valorHex(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static bool hexABytes(const string &hex, unsigned char *bytes, size_t longitud)
{
    if (hex.size() != longitud * 2) return false;
    for (size_t i = 0; i < longitud; i++)
    {
        int alto = valorHex(hex[i * 2]);
        int bajo = valorHex(hex[i * 2 + 1]);
        if (alto < 0 || bajo < 0) return false;
        bytes[i] = static_cast<unsigned char>((alto << 4) | bajo);
    }
    return true;
}

// --- Funciones públicas ---

string cifrarArchivo(const string &textoPlano, const unsigned char clave[32], const unsigned char salt[16], unsigned long long opslimit, size_t memlimit)
{
    if (sodium_init() < 0)
    {
        cerr << "No se pudo inicializar libsodium." << endl;
        return "";
    }

    if (crypto_aead_aes256gcm_is_available() != 1)
    {
        cerr << "AES-GCM no disponible en esta CPU." << endl;
        return "";
    }

    // Nonce aleatorio nuevo cada vez (requisito de AES-GCM).
    unsigned char nonce[crypto_aead_aes256gcm_NPUBBYTES];
    randombytes_buf(nonce, sizeof(nonce));

    // Cifrar. El resultado ocupa ABYTES (16) más que el texto: es el tag de autenticación.
    vector<unsigned char> cifrado(textoPlano.size() + crypto_aead_aes256gcm_ABYTES);
    unsigned long long tamanoCifrado = 0;
    crypto_aead_aes256gcm_encrypt(
        cifrado.data(), &tamanoCifrado,
        reinterpret_cast<const unsigned char *>(textoPlano.data()), textoPlano.size(),
        nullptr, 0,   // associated data (no lo usamos)
        nullptr,      // nsec (reservado, siempre NULL)
        nonce, clave);

    // Formato v2: v2|opslimit|memlimit|salt|nonce|cifrado (ops/mem en decimal, resto en hex)
    return "v2|" + to_string(opslimit) + "|" + to_string(memlimit)
         + "|" + bytesAHex(salt, crypto_pwhash_SALTBYTES)
         + "|" + bytesAHex(nonce, crypto_aead_aes256gcm_NPUBBYTES)
         + "|" + bytesAHex(cifrado.data(), tamanoCifrado);
}

bool descifrarArchivo(const string &contenido, const unsigned char clave[32], string &textoPlano)
{
    // Formato v2: v2|opslimit|memlimit|salt|nonce|cifrado (6 campos)
    stringstream flujo(contenido);
    string version, opsStr, memStr, saltHex, nonceHex, cifradoHex;
    getline(flujo, version, '|');
    getline(flujo, opsStr, '|');
    getline(flujo, memStr, '|');
    getline(flujo, saltHex, '|');
    getline(flujo, nonceHex, '|');
    getline(flujo, cifradoHex);

    if (version != "v2")
    {
        return false;
    }

    unsigned char nonce[crypto_aead_aes256gcm_NPUBBYTES];
    if (!hexABytes(nonceHex, nonce, sizeof(nonce)))
    {
        return false;
    }

    vector<unsigned char> cifrado(cifradoHex.size() / 2);
    if (!hexABytes(cifradoHex, cifrado.data(), cifrado.size()))
    {
        return false;
    }
    if (cifrado.size() < crypto_aead_aes256gcm_ABYTES)
    {
        return false;
    }

    // Descifrar: si el tag no cuadra (contraseña mal o archivo manipulado) devuelve != 0.
    vector<unsigned char> plano(cifrado.size() - crypto_aead_aes256gcm_ABYTES);
    unsigned long long tamanoPlano = 0;
    if (crypto_aead_aes256gcm_decrypt(
            plano.data(), &tamanoPlano,
            nullptr,                  // nsec (posición 3 en decrypt)
            cifrado.data(), cifrado.size(),
            nullptr, 0,               // associated data
            nonce, clave) != 0)
    {
        return false;
    }

    textoPlano.assign(reinterpret_cast<const char *>(plano.data()), tamanoPlano);
    return true;
}

void derivarClave(const string &password, const unsigned char salt[16], unsigned char clave[32], unsigned long long opslimit, size_t memlimit)
{
    // Argon2id convierte la contraseña en una clave de 32 bytes.
    // opslimit y memlimit controlan cuánto cuesta (más = más seguro = más lento).
    if (crypto_pwhash(clave, crypto_aead_aes256gcm_KEYBYTES,
                      password.c_str(), password.size(),
                      salt,
                      opslimit,
                      memlimit,
                      crypto_pwhash_ALG_ARGON2ID13) != 0)
    {
        cerr << "Error al derivar la clave." << endl;
    }
}

bool generarSalt(unsigned char salt[16])
{
    randombytes_buf(salt, crypto_pwhash_SALTBYTES);
    return true;
}

// Extrae solo el salt del contenido (para detectar si el archivo existe).
// v2: v2|opslimit|memlimit|salt|nonce|cifrado → salt está en posición 4.
bool extraerSaltDeContenido(const string &contenido, unsigned char salt[16])
{
    stringstream flujo(contenido);
    string version, opsStr, memStr, saltHex;
    getline(flujo, version, '|');
    getline(flujo, opsStr, '|');
    getline(flujo, memStr, '|');
    getline(flujo, saltHex, '|');
    if (version != "v2" || saltHex.empty())
    {
        return false;
    }
    return hexABytes(saltHex, salt, crypto_pwhash_SALTBYTES);
}

// Extrae opslimit y memlimit del formato v2: v2|opslimit|memlimit|...
bool extraerParametrosDeContenido(const string &contenido, unsigned long long &opslimit, size_t &memlimit)
{
    stringstream flujo(contenido);
    string version, opsStr, memStr;
    getline(flujo, version, '|');
    getline(flujo, opsStr, '|');
    getline(flujo, memStr, '|');

    if (version != "v2" || opsStr.empty() || memStr.empty())
    {
        return false;
    }

    opslimit = stoull(opsStr);
    memlimit = stoul(memStr);
    return true;
}
