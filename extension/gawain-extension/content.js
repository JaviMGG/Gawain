const Yvain = window.Yvain;

if (!Yvain) {
    console.error("Gawain: yvain-bundle.js no cargó correctamente");
} else {
    const dominio = window.location.hostname;

    if (dominio && dominio !== "") {
        Yvain.onLogin(({ dominio: d, contraseña }) => {
            if (!contraseña || contraseña === "") {
                return;
            }
            browser.runtime.sendMessage({
                action: "save",
                dominio: d,
                contraseña
            });
        });

        browser.runtime.sendMessage({ action: "get", dominio }).then((respuesta) => {
            if (respuesta && respuesta.ok && respuesta.contraseña) {
                Yvain.autofill({ dominio, contraseña: respuesta.contraseña });
            }
        });
    }
}
