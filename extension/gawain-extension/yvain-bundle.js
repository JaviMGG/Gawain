(function () {
    "use strict";

    const Yvain = (function () {
        const detector = (function () {
            function encontrarFormulario() {
                const campoPassword = document.querySelector('input[type="password"]');
                if (!campoPassword) {
                    return null;
                } else {
                    return campoPassword.closest('form');
                }
            }

            function extraerContraseña(form) {
                const input = form.querySelector('input[type="password"]');
                if (input) {
                    return input.value;
                } else {
                    return "";
                }
            }

            return { encontrarFormulario, extraerContraseña };
        })();

        const autofillMod = (function () {
            function autofill(datos) {
                if (!datos.dominio || datos.dominio !== window.location.hostname) {
                    return false;
                }

                const form = detector.encontrarFormulario();
                if (!form) {
                    return false;
                }

                const campoPassword = form.querySelector('input[type="password"]');
                if (!campoPassword || campoPassword.value !== '') {
                    return false;
                }

                campoPassword.value = datos.contraseña;
                return true;
            }

            return { autofill };
        })();

        const listaCallbacks = [];
        let listenerRegistrado = false;

        function manejadorSubmit(evento) {
            const form = evento.target.closest('form');
            if (!form) {
                return;
            }
            const contraseña = detector.extraerContraseña(form);
            const dominio = window.location.hostname;
            for (const callback of listaCallbacks) {
                callback({ dominio, contraseña });
            }
        }

        function registroUnicoListener() {
            if (!listenerRegistrado) {
                window.addEventListener('submit', manejadorSubmit);
                listenerRegistrado = true;
            }
        }

        function eliminarCallback(callback) {
            const indice = listaCallbacks.indexOf(callback);
            if (indice !== -1) {
                listaCallbacks.splice(indice, 1);
            }
        }

        function onLogin(callback) {
            if (!listaCallbacks.includes(callback)) {
                listaCallbacks.push(callback);
            }
            registroUnicoListener();
            return () => eliminarCallback(callback);
        }

        function detect(dominio) {
            let form = detector.encontrarFormulario();
            if (!form) {
                return null;
            }
            let contraseña = detector.extraerContraseña(form);
            if (contraseña == "") {
                return null;
            }
            return { dominio, contraseña };
        }

        return { detect, autofill: autofillMod.autofill, onLogin };
    })();

    window.Yvain = Yvain;
})();
