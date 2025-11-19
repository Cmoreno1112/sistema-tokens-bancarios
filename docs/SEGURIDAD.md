# Sistema de Seguridad

## Implementación de Token Dinámico

### Características del Token

1. **Formato**: 6 dígitos numéricos (100000-999999)
2. **Generación**: Pseudo-aleatoria usando Mersenne Twister
3. **Duración**: 60 segundos
4. **Uso**: One-Time Password (OTP)

### Algoritmo de Generación
```cpp
string generarToken() {
    random_device rd;                    // Semilla aleatoria
    mt19937 gen(rd());                   // Generador Mersenne Twister
    uniform_int_distribution<> dis(100000, 999999);  // Rango 6 dígitos
    return to_string(dis(gen));
}
```

## Flujo de Seguridad

### 1. Solicitud de Token