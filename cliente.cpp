#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <chrono>
#include <sstream>

using namespace std;

long long getTimestamp() {
    auto ahora = chrono::system_clock::now();
    auto duracion = ahora.time_since_epoch();
    return chrono::duration_cast<chrono::seconds>(duracion).count();
}

string enviarMensaje(const string& host, int puerto, const string& mensaje) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "Error al crear socket" << endl;
        return "";
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(puerto);
    
    if (inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr) <= 0) {
        cout << "Direccion no valida" << endl;
        return "";
    }
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cout << "Error de conexion" << endl;
        return "";
    }
    
    send(sock, mensaje.c_str(), mensaje.length(), 0);
    int valread = read(sock, buffer, 1024);
    
    close(sock);
    
    return string(buffer, valread);
}

string solicitarToken(const string& host, int puerto, const string& usuario) {
    string mensaje = "SOLICITAR_TOKEN|USUARIO:" + usuario;
    cout << "\n[CLIENTE] Solicitando token para usuario: " << usuario << endl;
    
    string respuesta = enviarMensaje(host, puerto, mensaje);
    
    if (respuesta.empty()) {
        cout << "[ERROR] No se pudo obtener respuesta del servidor" << endl;
        return "";
    }
    
    size_t pos = respuesta.find("TOKEN:");
    size_t fin = respuesta.find("|", pos);
    
    if (pos != string::npos && fin != string::npos) {
        string token = respuesta.substr(pos + 6, fin - pos - 6);
        
        pos = respuesta.find("EXPIRA:");
        fin = respuesta.find("|", pos);
        string expira = respuesta.substr(pos + 7, fin - pos - 7);
        
        cout << "[CLIENTE] Token recibido: " << token << endl;
        cout << "[CLIENTE] Valido por: " << expira << " segundos" << endl;
        
        return token;
    }
    
    return "";
}

void realizarTransaccion(const string& host, int puerto, 
                         const string& usuario, const string& destino,
                         double monto, const string& token) {
    
    long long timestamp = getTimestamp();
    
    stringstream mensaje;
    mensaje << "TRANS|USUARIO:" << usuario 
            << "|DESTINO:" << destino 
            << "|MONTO:" << monto
            << "|TOKEN:" << token
            << "|TIMESTAMP:" << timestamp;
    
    cout << "\n[CLIENTE] Enviando transaccion:" << endl;
    cout << "  De: " << usuario << endl;
    cout << "  Para: " << destino << endl;
    cout << "  Monto: $" << monto << endl;
    cout << "  Token: " << token << endl;
    
    string respuesta = enviarMensaje(host, puerto, mensaje.str());
    
    cout << "\n[SERVIDOR RESPONDIO]: " << respuesta << endl;
    
    if (respuesta.find("APROBADA") != string::npos) {
        cout << "OK TRANSACCION EXITOSA" << endl;
    } else {
        cout << "X TRANSACCION RECHAZADA" << endl;
    }
}

void menuInteractivo() {
    string host = "servidor";
    int puerto = 8080;
    string usuario;
    
    cout << "\n=== CLIENTE DE TRANSACCIONES ===" << endl;
    cout << "Ingrese su nombre de usuario: ";
    getline(cin, usuario);
    
    while (true) {
        cout << "\n--- MENU ---" << endl;
        cout << "1. Solicitar token" << endl;
        cout << "2. Realizar transaccion" << endl;
        cout << "3. Salir" << endl;
        cout << "Opcion: ";
        
        int opcion;
        cin >> opcion;
        cin.ignore();
        
        if (opcion == 1) {
            string token = solicitarToken(host, puerto, usuario);
            if (!token.empty()) {
                cout << "\nGuarde este token para su transaccion." << endl;
            }
        }
        else if (opcion == 2) {
            string destino, token;
            double monto;
            
            cout << "Destinatario: ";
            getline(cin, destino);
            
            cout << "Monto: $";
            cin >> monto;
            cin.ignore();
            
            cout << "Token: ";
            getline(cin, token);
            
            realizarTransaccion(host, puerto, usuario, destino, monto, token);
        }
        else if (opcion == 3) {
            cout << "Saliendo..." << endl;
            break;
        }
        else {
            cout << "Opcion no valida" << endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        menuInteractivo();
        return 0;
    }
    
    if (argc < 5) {
        cout << "Uso: " << argv[0] << " <usuario> <destino> <monto> <servidor_host>" << endl;
        cout << "O ejecute sin argumentos para modo interactivo" << endl;
        return 1;
    }
    
    string usuario = argv[1];
    string destino = argv[2];
    double monto = stod(argv[3]);
    string host = argv[4];
    int puerto = 8080;
    
    string token = solicitarToken(host, puerto, usuario);
    
    if (token.empty()) {
        cout << "No se pudo obtener token" << endl;
        return 1;
    }
    
    sleep(2);
    
    realizarTransaccion(host, puerto, usuario, destino, monto, token);
    
    return 0;
}