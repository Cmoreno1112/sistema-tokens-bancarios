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

struct TokenInfo {
    string token;
    long long timestamp;
    int duracion_segundos;
};

map<string, TokenInfo> tokens_activos;

string generarToken() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(100000, 999999);
    return to_string(dis(gen));
}

long long getTimestamp() {
    auto ahora = chrono::system_clock::now();
    auto duracion = ahora.time_since_epoch();
    return chrono::duration_cast<chrono::seconds>(duracion).count();
}

bool validarToken(const string& usuario, const string& token) {
    if (tokens_activos.find(usuario) == tokens_activos.end()) {
        return false;
    }
    
    TokenInfo& info = tokens_activos[usuario];
    long long tiempo_actual = getTimestamp();
    
    if (info.token == token && 
        (tiempo_actual - info.timestamp) <= info.duracion_segundos) {
        return true;
    }
    
    return false;
}

string procesarSolicitudToken(const string& usuario) {
    string token = generarToken();
    long long timestamp = getTimestamp();
    int duracion = 60;
    
    TokenInfo info = {token, timestamp, duracion};
    tokens_activos[usuario] = info;
    
    stringstream respuesta;
    respuesta << "TOKEN:" << token 
              << "|EXPIRA:" << duracion 
              << "|TIMESTAMP:" << timestamp;
    
    cout << "[SERVIDOR] Token generado para " << usuario 
         << ": " << token << " (valido " << duracion << "s)" << endl;
    
    return respuesta.str();
}

string procesarTransaccion(const string& datos) {
    map<string, string> campos;
    stringstream ss(datos);
    string item;
    
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
    
    if (validarToken(usuario, token)) {
        cout << "  OK Token valido - Transaccion APROBADA" << endl;
        tokens_activos.erase(usuario);
        
        return "APROBADA|ID:" + to_string(getTimestamp()) + 
               "|MONTO:" + monto + "|DESTINO:" + destino;
    } else {
        cout << "  X Token invalido o expirado - Transaccion RECHAZADA" << endl;
        return "RECHAZADA|MOTIVO:Token invalido o expirado";
    }
}

int main() {
    int servidor_fd, nuevo_socket;
    struct sockaddr_in direccion;
    int opt = 1;
    int addrlen = sizeof(direccion);
    char buffer[1024] = {0};
    
    if ((servidor_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error al crear socket");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(servidor_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt))) {
        perror("Error en setsockopt");
        exit(EXIT_FAILURE);
    }
    
    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port = htons(8080);
    
    if (bind(servidor_fd, (struct sockaddr *)&direccion, sizeof(direccion)) < 0) {
        perror("Error en bind");
        exit(EXIT_FAILURE);
    }
    
    if (listen(servidor_fd, 3) < 0) {
        perror("Error en listen");
        exit(EXIT_FAILURE);
    }
    
    cout << "=== SERVIDOR DE TRANSACCIONES INICIADO ===" << endl;
    cout << "Escuchando en puerto 8080..." << endl << endl;
    
    while (true) {
        cout << "Esperando conexion..." << endl;
        
        if ((nuevo_socket = accept(servidor_fd, (struct sockaddr *)&direccion,
                                   (socklen_t*)&addrlen)) < 0) {
            perror("Error en accept");
            continue;
        }
        
        cout << "Cliente conectado!" << endl;
        
        int valread = read(nuevo_socket, buffer, 1024);
        string solicitud(buffer, valread);
        
        cout << "Solicitud recibida: " << solicitud << endl;
        
        string respuesta;
        
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
        
        send(nuevo_socket, respuesta.c_str(), respuesta.length(), 0);
        cout << "Respuesta enviada: " << respuesta << endl << endl;
        
        close(nuevo_socket);
        memset(buffer, 0, sizeof(buffer));
    }
    
    return 0;
}