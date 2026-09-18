const HOST_NAME = "gawain-host";

console.log("Gawain: background iniciado, probando ", HOST_NAME, "...");
enviarAlHost({ op: "ping" }).then((r) => {
    console.log("Gawain: ping OK ->", JSON.stringify(r));
}).catch((err) => {
    const detalle = (err && err.message) ? err.message : String(err);
    console.log("Gawain: ping FALLO ->", detalle);
});

function enviarAlHost(mensaje) {
    return new Promise((resolve, reject) => {
        const puerto = browser.runtime.connectNative(HOST_NAME);

        puerto.onMessage.addListener((respuesta) => {
            puerto.disconnect();
            resolve(respuesta);
        });

        puerto.onDisconnect.addListener(() => {
            if (browser.runtime.lastError) {
                reject(browser.runtime.lastError);
            } else {
                reject(new Error("gawain-host desconectado"));
            }
        });

        puerto.postMessage(mensaje);
    });
}

function notificar(titulo, mensaje) {
    browser.notifications.create("gawain-" + Date.now(), {
        type: "basic",
        iconUrl: "icon-48.png",
        title: titulo,
        message: mensaje
    });
}

browser.runtime.onMessage.addListener((mensaje, remitente) => {
    if (!mensaje || typeof mensaje.action !== "string") {
        return;
    }

    if (mensaje.action === "save") {
        const { dominio, contraseña } = mensaje;
        if (!dominio || !contraseña) {
            return Promise.resolve({ ok: false, error: "Faltan datos" });
        }
        return enviarAlHost({
            op: "save",
            app: dominio,
            password: contraseña
        }).then((respuesta) => {
            if (respuesta.ok) {
                notificar("Gawain", "Contraseña guardada para " + dominio);
            } else {
                notificar("Gawain", "No se pudo guardar: " + (respuesta.error || "error"));
            }
            return respuesta;
        }).catch((err) => {
            const detalle = (err && err.message) ? err.message : String(err);
            console.error("Gawain: connectNative falló:", detalle);
            notificar("Gawain", "gawain-host no disponible: " + detalle);
            return { ok: false, error: detalle };
        });
    }

    if (mensaje.action === "get") {
        const { dominio } = mensaje;
        if (!dominio) {
            return Promise.resolve({ ok: false, error: "Falta dominio" });
        }
        return enviarAlHost({
            op: "get",
            app: dominio
        }).then((respuesta) => {
            if (respuesta.ok) {
                return { ok: true, contraseña: respuesta.password };
            }
            return { ok: false, error: respuesta.error };
        }).catch((err) => {
            return { ok: false, error: String(err) };
        });
    }

    if (mensaje.action === "ping") {
        return enviarAlHost({ op: "ping" }).then((r) => {
            console.log("Gawain: ping a gawain-host -> ok:", r.ok);
            return r;
        }).catch((err) => {
            const detalle = (err && err.message) ? err.message : String(err);
            console.error("Gawain: ping falló:", detalle);
            return { ok: false, error: detalle };
        });
    }

    return undefined;
});
