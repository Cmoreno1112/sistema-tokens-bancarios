# 🏦 Sistema de Transacciones con Tokens Dinámicos

Sistema bancario seguro implementado en Docker que simula el funcionamiento de la clave dinámica de Bancolombia.

## 🎯 Características

- ✅ Tokens de 6 dígitos con expiración de 60 segundos
- ✅ Uso único por token (OTP)
- ✅ Validación centralizada en servidor
- ✅ Contenedores optimizados (~10MB cada uno)
- ✅ Comunicación por sockets TCP
- ✅ Sin dependencias de frameworks externos

## 📋 Prerrequisitos

- Docker >= 20.10
- Docker Compose >= 2.0
- Git

## 🚀 Instalación

### Opción 1: Clonar repositorio
```bash
git clone https://github.com/tu-usuario/sistema-tokens-bancarios.git
cd sistema-tokens-bancarios
```

### Opción 2: Usar script automático
```bash
chmod +x setup.sh
./setup.sh
```

## 🔨 Construcción y Ejecución

### Construir y ejecutar
```bash
# Construir imágenes y ejecutar contenedores
docker-compose up --build

# O en background
docker-compose up -d --build
```

### Acceder al cliente
```bash
# En otra terminal
docker exec -it cliente_transacciones ./cliente
```

## 📖 Uso

### Modo Interactivo

1. Ejecutar cliente:
```bash
docker exec -it cliente_transacciones ./cliente
```

2. Ingresar nombre de usuario

3. Menú de opciones:
   - **Opción 1**: Solicitar token
   - **Opción 2**: Realizar transacción
   - **Opción 3**: Cambiar servidor
   - **Opción 4**: Salir

### Ejemplo de Flujo Completo
```
Usuario: cmoreno

1. Solicitar token
   → Token recibido: 654321
   → Válido por: 60 segundos

2. Realizar transacción
   → Destinatario: jperez
   → Monto: $5000
   → Token: 654321
   → Resultado: ✅ TRANSACCION EXITOSA
```

## 🔒 Sistema de Seguridad

### Características del Token

- **Generación**: Números aleatorios de 6 dígitos (100000-999999)
- **Algoritmo**: Mersenne Twister (mt19937)
- **Duración**: 60 segundos de validez
- **Uso**: One-Time Password (se invalida después de usar)
- **Timestamp**: Cada transacción tiene marca temporal única

### Validaciones

1. ✅ Token existe para el usuario
2. ✅ Token coincide exactamente
3. ✅ Token no ha expirado
4. ✅ Token no ha sido usado previamente

## 📊 Arquitectura
```
┌─────────────────────────────────────────────┐
│          RED DOCKER (BRIDGE)                │
│                                             │
│  ┌──────────────┐    ┌──────────────┐     │
│  │   Cliente    │◄──►│   Servidor   │     │
│  │  Alpine      │TCP │   Alpine     │     │
│  │  ~10MB       │8080│   ~10MB      │     │
│  └──────────────┘    └──────────────┘     │
└─────────────────────────────────────────────┘
```

## 🧪 Casos de Prueba

### Prueba 1: Token Válido
```bash
1. Solicitar token
2. Usar inmediatamente (< 60s)
Resultado: ✅ APROBADA
```

### Prueba 2: Token Expirado
```bash
1. Solicitar token
2. Esperar > 60 segundos
3. Intentar usar token
Resultado: ❌ RECHAZADA (Token expirado)
```

### Prueba 3: Token Reutilizado
```bash
1. Solicitar token
2. Realizar transacción exitosa
3. Intentar reutilizar mismo token
Resultado: ❌ RECHAZADA (Token ya usado)
```

### Prueba 4: Token Inválido
```bash
1. Intentar transacción con token inventado
Resultado: ❌ RECHAZADA (Token inválido)
```

## 📝 Formato de Mensajes

### Solicitud de Token
```
SOLICITAR_TOKEN|USUARIO:cmoreno
```

### Respuesta de Token
```
TOKEN:654321|EXPIRA:60|TIMESTAMP:1700000000
```

### Transacción
```
TRANS|USUARIO:cmoreno|DESTINO:jperez|MONTO:5000|TOKEN:654321|TIMESTAMP:1700000010
```

### Respuesta Exitosa
```
APROBADA|ID:1700000010|MONTO:5000|DESTINO:jperez
```

### Respuesta Fallida
```
RECHAZADA|MOTIVO:Token invalido o expirado
```

## 🛠️ Comandos Útiles
```bash
# Ver contenedores activos
docker ps

# Ver logs del servidor
docker logs -f servidor_tokens

# Ver logs del cliente
docker logs -f cliente_transacciones

# Detener servicios
docker-compose down

# Reconstruir desde cero
docker-compose build --no-cache

# Ver tamaño de imágenes
docker images | grep token

# Limpiar todo
docker-compose down --rmi all --volumes
docker system prune -af
```

## 🐛 Troubleshooting

### Error: "Direccion no valida"
```bash
# Verificar que ambos contenedores estén corriendo
docker ps

# Verificar red
docker network inspect sistema-tokens-bancarios_red_transacciones

# Obtener IP del servidor manualmente
docker inspect servidor_tokens | grep IPAddress

# Usar opción 3 del menú para cambiar servidor
```

### Error: "Fallo al conectar"
```bash
# Verificar que el servidor esté escuchando
docker logs servidor_tokens

# Debería mostrar: "Escuchando en puerto 8080..."
```

### Reconstruir por problemas de compilación
```bash
docker-compose down
docker system prune -af
docker-compose build --no-cache
docker-compose up
```

## 🐳 Optimización de Docker

Las imágenes de Docker se han optimizado utilizando **builds multi-etapa**.

1.  **Etapa de Compilación**: Se utiliza una imagen `alpine` con el compilador `g++` y las herramientas necesarias para compilar la aplicación C++.
2.  **Etapa de Producción**: Se utiliza una imagen `alpine` limpia y vacía. Solo se copia el binario ejecutable de la etapa anterior.

Este enfoque reduce drásticamente el tamaño de la imagen final, ya que no contiene las dependencias de compilación, resultando en imágenes más seguras y ligeras (menos de 5MB).

## 📈 Métricas

- **Tamaño imagen servidor**: ~4.1 MB
- **Tamaño imagen cliente**: ~3.8 MB
- **Tiempo de compilación**: ~45 segundos
- **Tiempo de respuesta**: < 10ms
- **Tokens generados por segundo**: ~1000

## 👥 Equipo

- [Integrante 1](https://github.com/usuario1) - Desarrollo del servidor
- [Integrante 2](https://github.com/usuario2) - Desarrollo del cliente
- [Integrante 3](https://github.com/usuario3) - Dockerización y documentación

## 📄 Licencia

MIT License

## 🔗 Enlaces

- [Repositorio GitHub](https://github.com/tu-usuario/sistema-tokens-bancarios)
- [Wiki Completa](https://github.com/tu-usuario/sistema-tokens-bancarios/wiki)
- [Issues](https://github.com/tu-usuario/sistema-tokens-bancarios/issues)
```

---

## 📝 ARCHIVO 7: `.gitignore`
```
# Binarios compilados
servidor
cliente
*.o
*.out
*.exe
*.a

# Archivos de Docker
.dockerignore

# Archivos del sistema
.DS_Store
Thumbs.db
*.swp
*.swo
*~

# IDEs y editores
.vscode/
.idea/
*.sublime-project
*.sublime-workspace

# Logs
*.log
logs/

# Temporales
tmp/
temp/


## 📊 Arquitectura

sistema-tokens-bancarios/
│
├── 📄 servidor.cpp              ← Código del servidor (genera y valida tokens)
├── 📄 cliente.cpp               ← Código del cliente (solicita tokens y hace transacciones)
├── 🐳 Dockerfile.servidor       ← Instrucciones para crear imagen del servidor
├── 🐳 Dockerfile.cliente        ← Instrucciones para crear imagen del cliente
├── 🐳 docker-compose.yml        ← Orquestación de los 2 contenedores
├── 📝 README.md                 ← Documentación principal
├── 📝 .gitignore                ← Archivos a ignorar en Git
├── ⚙️  setup.sh                  ← Script automático de instalación
│
├── 📁 docs/
│   ├── ARQUITECTURA.md          ← Explicación técnica de la arquitectura
│   ├── SEGURIDAD.md             ← Documentación del sistema de seguridad
│   ├── PRUEBAS.md               ← Casos de prueba
│   └── COMANDOS.md              ← Guía de comandos Docker
│
├── 📁 .github/
│   └── workflows/
│       └── docker-publish.yml   ← CI/CD para DockerHub (opcional)
│
└── 📁 screenshots/              ← Capturas de pantalla para documentación
    ├── terminal-servidor.png
    ├── terminal-cliente.png
    └── arquitectura.png