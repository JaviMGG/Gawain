# Gawain

Gestor de contraseñas de escritorio escrito en C++ como proyecto educativo.

> ⚠️ **Aviso**: es un proyecto de aprendizaje, no una herramienta auditada. No lo uses para guardar contraseñas reales.

## Características

- Generación aleatoria de contraseñas (40 caracteres alfanuméricos)
- Almacenamiento cifrado con [libsodium](https://libsodium.org/)
  - Derivación de clave: **Argon2id** (MODERATE)
  - Cifrado autenticado: **AES-256-GCM**
- Formato v2: los parámetros de derivación (`opslimit`, `memlimit`) viajan en la cabecera del archivo, así el coste puede subir en el futuro sin romper bases de datos antiguas
- Contraseña maestra confirmada dos veces en la primera ejecución y oculta durante la introducción
- Clave y contraseña borradas de memoria al salir (`sodium_memzero`)
- Permisos `0600` en la base de datos

## Fases del proyecto

### Fase 1 — Gestor en consola
La base del programa: un `map<string, vector<string>>` que asocia cada aplicación con su lista de contraseñas, y un menú por terminal para generar, listar, buscar y eliminar entradas.

### Fase 2 — Generador de contraseñas
Generador aleatorio basado en `<random>` (`mt19937` + `uniform_int_distribution`) sobre un alfabeto alfanumérico, devolviendo cadenas de 40 caracteres.

### Fase 3 — Cifrado real con libsodium
El paso de "archivo de texto plano" a "base de datos cifrada":
- Se separa el proyecto en módulos: lógica (`gawain.cpp/h`), persistencia (`persistence.cpp`) y cifrado (`cifrado.cpp/h`).
- La contraseña maestra no se usa directamente: se deriva una clave de 32 bytes con **Argon2id** y un salt aleatorio de 16 bytes.
- El contenido se cifra con **AES-256-GCM**, que además de cifrar firma los datos: si el archivo se manipula o la contraseña es incorrecta, el descifrado falla.
- La contraseña se introduce oculta (apagando el eco de la terminal con `termios`).

### Fase 4 — Endurecimiento
Cinco mejoras de seguridad sobre la Fase 3:
1. **Confirmar contraseña maestra** dos veces en la primera ejecución, para no perder acceso por una errata.
2. **Limpieza de memoria**: `sodium_memzero` borra la contraseña tras derivar la clave, y la clave al salir del programa.
3. **Formato v2**: la cabecera del archivo pasa a ser `v2|opslimit|memlimit|salt|nonce|cifrado`. Los parámetros de Argon2id viajan dentro del propio archivo, de modo que subir el coste de derivación en el futuro no rompe las bases de datos antiguas.
4. **Retardo** de 2 segundos cuando la contraseña maestra es incorrecta.
5. **Permisos 0600** en la base de datos (solo el propietario puede leerla/escribirla).

### Fase 5 — Interfaz gráfica con Qt 🚧
Migración de la consola a una ventana de escritorio con Qt Widgets:
- ✅ **Refactor "cerebro sin boca"**: las funciones de lógica ya no imprimen nada; devuelven datos (`string`/`bool`) y es quien llama quien decide cómo mostrarlos. Condición previa para poder conectar cualquier interfaz.
- ✅ **Proyecto CMake + primera ventana**: migrado a `CMakeLists.txt` con Qt6, clase `MainWindow` (hereda de `QMainWindow`) y bucle de eventos `QApplication::exec()`.
- ⬜ Layout de botones y campos (generar, listar, buscar, eliminar).
- ⬜ Conectar cada botón al módulo lógico mediante señales/slots.
- ⬜ Tema oscuro.

### Fase 6 — Publicación 🚧
Publicación del código en GitHub bajo licencia MIT, con este README y sin datos sensibles en el repositorio.

## Compilación

Requisitos: compilador C++17, CMake, Qt 6 (Widgets) y libsodium.

```bash
cmake -B build
cmake --build build
./build/gawain
```

## Uso

1. La primera vez pide crear una contraseña maestra (se pide dos veces para confirmar).
2. Desde la interfaz puedes generar, listar, buscar y eliminar contraseñas por aplicación.
3. Todo se guarda cifrado en `archivo.txt` junto al ejecutable.
4. En siguientes ejecuciones solo se pide la contraseña maestra; si es incorrecta hay un retardo antes de salir.

## Formato de la base de datos

```
v2|opslimit|memlimit|salt|nonce|ciphertext
   decimal |decimal| hex |hex |    hex
```

- `opslimit` / `memlimit`: coste de Argon2id usado al crear ese archivo.
- `salt`: 16 bytes que garantizan claves distintas para la misma contraseña.
- `nonce`: 12 bytes aleatorios por operación de cifrado.
- `ciphertext`: datos cifrados + tag de autenticación GCM (16 bytes).

## Estructura

```
main.cpp         arranque: desbloqueo + QApplication
main_window.*    ventana principal (Qt)
gawain.cpp/h     lógica: gestor de contraseñas ("cerebro", sin I/O)
persistence.cpp  guardar/cargar la base de datos
cifrado.cpp/h    libsodium: Argon2id + AES-256-GCM, formato v2
```

## Licencia

MIT — ver [LICENSE](LICENSE).
