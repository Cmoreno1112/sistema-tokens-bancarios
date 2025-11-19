# 🏦 Sistema de Transacciones con Tokens Dinámicos

Sistema bancario seguro implementado en Docker que simula el funcionamiento de la clave dinámica de Bancolombia.

## 🎯 Características

- ✅ Tokens de 6 dígitos con expiración de 60 segundos
- ✅ Uso único por token (OTP)
- ✅ Validación en servidor
- ✅ Contenedores optimizados (~10MB cada uno)
- ✅ Comunicación por sockets TCP
- ✅ Sin dependencias de frameworks externos

## 🚀 Inicio Rápido

### Prerrequisitos
- Docker
- Docker Compose

### Instalación

1. Clonar el repositorio:
```bash
git clone https://github.com/tu-usuario/sistema-tokens-bancarios.git
cd sistema-tokens-bancarios
```

2. Construir y ejecutar:
```bash
docker-compose up --build
```

3. En otra terminal, acceder al cliente:
```bash
docker exec -it cliente_transacciones ./cliente
```

## 📖 Uso

### Modo Interactivo
```bash
docker exec -it cliente_transacciones ./cliente
```

Sigue el menú:
1. Solicitar token
2. Realizar transacción (usando el token obtenido)
3. Salir

### Modo Comando
```bash
docker exec cliente_transacciones ./cliente juan maria 5000 servidor
```

## 🔒 Sistema de Seguridad

- **Generación de tokens**: Números aleatorios de 6 dígitos
- **Expiración temporal**: 60 segundos de validez
- **One-Time Password**: El token se invalida después de usarse
- **Timestamp único**: Cada transacción tiene marca temporal
- **Validación centralizada**: Solo el servidor valida tokens

## 📊 Arquitectura