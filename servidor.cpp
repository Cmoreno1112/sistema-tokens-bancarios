#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <map>

using namespace std;

/**
 * @struct TokenInfo
 * @brief Almacena la información de un token generado.
 *
 * Contiene el token, el momento de su creación (timestamp) y su duración.
 */
struct TokenInfo {
    string token;
    long long timestamp;
    int duracion_segundos;
};

// Mapa para almacenar los tokens activos, usando el nombre de usuario como clave.
map<string, TokenInfo> tokens_activos;

/**
 * @brief Genera un token numérico aleatorio de 6 dígitos.
 * @return Una cadena con el token generado.
 */
string generarToken() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(100000, 999999);
    return to_string(dis(gen));
}

/**
 * @brief Obtiene el timestamp actual en segundos desde la Época (Epoch).
 * @return Un valor long long representando el timestamp.
 */
long long getTimestamp() {
    auto ahora = chrono::system_clock::now();
    auto duracion = ahora.time_since_epoch();
    return chrono::duration_cast<chrono::seconds>(duracion).count();
}

/**
 * @brief Valida si un token proporcionado por un usuario es correcto y no ha expirado.
 * @param usuario El nombre del usuario.
 * @param token El token a validar.
 * @return `true` si el token es válido, `false` en caso contrario.
 */
bool validarToken(const string& usuario, const string& token) {
    // Verifica si el usuario tiene un token activo.
    if (tokens_activos.find(usuario) == tokens_activos.end()) {
        return false;
    }
    
    TokenInfo& info = tokens_activos[usuario];
    long long tiempo_actual = getTimestamp();
    
    // Comprueba que el token coincida y que no haya pasado su tiempo de vida.
    if (info.token == token && 
        (tiempo_actual - info.timestamp) <= info.duracion_segundos) {
        return true;
    }
    
    return false;
}

/**
 * @brief Procesa una solicitud para generar un nuevo token para un usuario.
 * @param usuario El nombre del usuario que solicita el token.
 * @return Una cadena con el token, su duración y el timestamp.
 */
string procesarSolicitudToken(const string& usuario) {
    string token = generarToken();
    long long timestamp = getTimestamp();
    int duracion = 60; // El token es válido por 60 segundos.
    
    TokenInfo info = {token, timestamp, duracion};
    tokens_activos[usuario] = info; // Almacena o actualiza el token del usuario.
    
    stringstream respuesta;
    respuesta << "TOKEN:" << token 
              << "|EXPIRA:" << duracion 
              << "|TIMESTAMP:" << timestamp;
    
    cout << "[SERVIDOR] Token generado para " << usuario 
         << ": " << token << " (valido " << duracion << "s)" << endl;
    
    return respuesta.str();
}

/**
 * @brief Procesa una solicitud de transacción, validando el token.
 * @param datos La cadena de datos recibida del cliente.
 * @return Una cadena indicando si la transacción fue APROBADA o RECHAZADA.
 */
string procesarTransaccion(const string& datos) {
    map<string, string> campos;
    stringstream ss(datos);
    string item;
    
    // Parsea la cadena de datos (ej: "TRANS|USUARIO:xyz|TOKEN:123...")
    while (getline(ss, item, '|')) {
        size_t pos = item.find(':');
        if (pos != string::npos) {
            string clave = item.substr(0, pos);
            string valor = item.substr(pos + 1);
            campos[clave] = valor;
        }
    }
    
    string usuario = campos["USUARIO"];
    string token = campos["TOKEN"];
    string destino = campos["DESTINO"];
    string monto = campos["MONTO"];
    string timestamp = campos["TIMESTAMP"];
    
    cout << "[SERVIDOR] Procesando transaccion:" << endl;
    cout << "  Usuario: " << usuario << endl;
    cout << "  Destino: " << destino << endl;
    cout << "  Monto: $" << monto << endl;
    cout << "  Token: " << token << endl;
    
    // Si el token es válido, la transacción se aprueba y el token se consume.
    if (validarToken(usuario, token)) {
        cout << "  OK Token valido - Transaccion APROBADA" << endl;
        tokens_activos.erase(usuario); // El token se usa una sola vez.
        
        return "APROBADA|ID:" + to_string(getTimestamp()) + 
               "|MONTO:" + monto + "|DESTINO:" + destino;
    } else {
        cout << "  X Token invalido o expirado - Transaccion RECHAZADA" << endl;
        return "RECHAZADA|MOTIVO:Token invalido o expirado";
    }
}

/**
 * @brief Función principal que inicia el servidor.
 *
 * Crea un socket, lo asocia a un puerto y se queda escuchando por conexiones
 * entrantes para procesar solicitudes de token o transacciones.
 */
int main() {
    int servidor_fd, nuevo_socket;
    struct sockaddr_in direccion;
    int opt = 1;
    int addrlen = sizeof(direccion);
    char buffer[1024] = {0};
    
    // Crear el descriptor del socket del servidor.
    if ((servidor_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error al crear socket");
        exit(EXIT_FAILURE);
    }
    
    // Configurar el socket para reutilizar la dirección y el puerto.
    if (setsockopt(servidor_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt))) {
        perror("Error en setsockopt");
        exit(EXIT_FAILURE);
    }
    
    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY; // Aceptar conexiones de cualquier IP.
    direccion.sin_port = htons(8080);       // Puerto de escucha.
    
    // Asociar el socket a la dirección y puerto.
    if (bind(servidor_fd, (struct sockaddr *)&direccion, sizeof(direccion)) < 0) {
        perror("Error en bind");
        exit(EXIT_FAILURE);
    }
    
    // Poner el servidor en modo escucha.
    if (listen(servidor_fd, 3) < 0) {
        perror("Error en listen");
        exit(EXIT_FAILURE);
    }
    
    cout << "=== SERVIDOR DE TRANSACCIONES INICIADO ===" << endl;
    cout << "Escuchando en puerto 8080..." << endl << endl;
    
    // Bucle infinito para aceptar y procesar conexiones.
    while (true) {
        cout << "Esperando conexion..." << endl;
        
        // Aceptar una nueva conexión de un cliente.
        if ((nuevo_socket = accept(servidor_fd, (struct sockaddr *)&direccion,
                                   (socklen_t*)&addrlen)) < 0) {
            perror("Error en accept");
            continue;
        }
        
        cout << "Cliente conectado!" << endl;
        
        // Leer la solicitud del cliente.
        int valread = read(nuevo_socket, buffer, 1024);
        string solicitud(buffer, valread);
        
        cout << "Solicitud recibida: " << solicitud << endl;
        
        string respuesta;
        
        // Determinar el tipo de solicitud y procesarla.
        if (solicitud.find("SOLICITAR_TOKEN") == 0) {
            size_t pos = solicitud.find("|USUARIO:");
            string usuario = solicitud.substr(pos + 9);
            respuesta = procesarSolicitudToken(usuario);
        } 
        else if (solicitud.find("TRANS") == 0) {
            respuesta = procesarTransaccion(solicitud);
        }
        else {
            respuesta = "ERROR|Solicitud no reconocida";
        }
        
        // Enviar la respuesta al cliente.
        send(nuevo_socket, respuesta.c_str(), respuesta.length(), 0);
        cout << "Respuesta enviada: " << respuesta << endl << endl;
        
        // Cerrar la conexión con el cliente actual.
        close(nuevo_socket);
        memset(buffer, 0, sizeof(buffer));
    }
    
    return 0;
}