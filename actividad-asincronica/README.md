Componente: RateLimiter (Control de Tasa por Token Bucket)

Propuesta de un nuevo componente independiente para el modelo de componentes en C++, diseñado para regular la frecuencia de solicitudes (por ejemplo, en un servidor HTTP o gateway interno) sin acoplarse a la lógica de red ni al almacenamiento específico del Host.

1. Justificación como Componente Independiente

De acuerdo con la teoría de componentes (Sommerville) y el modelo de componentes en C++ del curso, RateLimiter justifica ser un componente autónomo y reutilizable por los siguientes motivos:

Independencia y encapsulamiento: Encapsula el algoritmo de control de tasa (Token Bucket) y el estado por clave/cliente dentro del propio módulo dinámico (.dll / .so / .dylib), exponiendo únicamente operaciones parametrizadas por interfaz.

Reutilización entre contextos: Puede cargarse dinámicamente desde un servidor HTTP, un worker de eventos o un CLI sin recompilar el Host ni conocer la implementación interna.

Sustituibilidad y evolución: Permite reemplazar o actualizar la estrategia de limitación (por ejemplo, distintos parámetros o políticas) respetando el contrato de interfaz y el control de versión del ABI (CURRENT_API_VERSION).

Alineación con nomenclatura y principios C++: Sigue las convenciones híbridas del proyecto (snake_case para archivos, funciones, variables y parámetros; PascalCase para clases, structs y enums; sufijo _ en atributos privados; UPPER_SNAKE_CASE para constantes; RAII; SRP; e inmutabilidad por defecto con const / const ...&).

2. Requerimientos Funcionales

RF-01 — Evaluación de permiso por clave: El componente debe evaluar si una solicitud identificada por una clave (por ejemplo, IP de origen, token de cliente o identificador de recurso) puede continuar o debe ser rechazada según los tokens disponibles en su cubo.

RF-02 — Recarga temporal de tokens: El componente debe reponer tokens de forma progresiva en función del tiempo transcurrido y de una tasa configurada, respetando una capacidad máxima por clave.

RF-03 — Consumo atómico de tokens: Al aprobar una solicitud, el componente debe descontar la cantidad requerida de tokens de manera consistente para evitar consumos inválidos o estados inconsistentes.

RF-04 — Consulta de estado / reintento: El componente debe poder informar el estado de la evaluación (permitido, límite excedido, argumento inválido o error interno) y, cuando aplique, un tiempo estimado de espera o tokens restantes mediante parámetros de salida seguros.

RF-05 — Reinicio o limpieza por clave: El componente debe permitir restablecer el estado de una clave específica cuando el sistema Host lo requiera (por ejemplo, por liberación manual o prueba).

3. Requerimientos No Funcionales y de Interfaz / ABI

RNF-01 — Frontera ABI segura (C-API y noexcept): Las funciones exportadas para el ciclo de vida y consulta de versión (get_api_version, create_component, destroy_component) deben declararse bajo extern "C" y marcarse como noexcept para evitar name mangling incompatible y propagación de excepciones a través del límite binario.

RNF-02 — Prohibición de tipos STL y excepciones en la frontera: Las interfaces públicas (i_component.hpp, i_rate_limiter.hpp) no deben exponer std::string, std::vector ni lanzar excepciones hacia el Host; deben emplear tipos primitivos/estilo C (const char*, size_t, uint64_t) y códigos de estado explícitos mediante enum class ComponentResult : int.

RNF-03 — Contención interna de excepciones: Toda operación interna que pueda lanzar excepciones (por ejemplo, asignación de memoria o manejo de reloj/contenedores internos) debe capturarse dentro del componente mediante try / catch (...) y traducirse a un ComponentResult (como ERROR_INTERNAL) o nullptr en la fábrica.

RNF-04 — Gestión de memoria y ciclo de vida con RAII y Custom Deleter: La creación y destrucción de instancias deben ocurrir en el heap del propio módulo mediante sus funciones exportadas, gestionadas en el Host con std::shared_ptr y un custom deleter que capture el std::shared_ptr<SharedLibrary> para impedir la descarga prematura de la biblioteca dinámica.

RNF-05 — Seguridad de hilos y concurrencia: Dado que múltiples hilos del Host (por ejemplo, hilos que atienden peticiones HTTP) pueden consultar el limitador simultáneamente, el acceso al estado interno de cubos debe sincronizarse con std::mutex y std::lock_guard (o std::unique_lock), aplicando inmutabilidad por defecto (const y referencias constantes donde corresponda).

RNF-06 — Ausencia de std::exit en infraestructura: Ni el componente ni la infraestructura de carga deben invocar std::exit; los errores de carga, símbolos faltantes o incompatibilidad de versión de ABI deben gestionarse mediante excepciones controladas en la capa de infraestructura/Host para preservar el stack unwinding y los destructores RAII.

RNF-07 — Control de versión de ABI: El componente debe exponer su versión de contrato (CURRENT_API_VERSION) para que el gestor de módulos valide compatibilidad antes de instanciar y evitar lecturas inválidas de vtable.

4. Estructura Propuesta del Componente (Nomenclatura C++)

Cabeceras: i_component.hpp, i_rate_limiter.hpp, shared_library.hpp, module_manager.hpp

Implementación del componente: rate_limiter_component.hpp, rate_limiter_component.cpp

Punto de entrada del Host: main.cpp

Tipos y miembros ilustrativos:

Interfaz / clase: IRateLimiter, RateLimiterComponent, TokenBucketState

Métodos: allow_request(...), reset_key(...), get_api_version(), create_component(), destroy_component(...)

Atributos privados de ejemplo: max_tokens_, refill_rate_, buckets_, state_mutex_

Constantes de ejemplo: CURRENT_API_VERSION, DEFAULT_MAX_TOKENS

5. Puntos Abiertos para el Análisis de Viabilidad y Factibilidad en Clase

Almacenamiento en memoria vs. compartido: ¿Es suficiente mantener el estado de los cubos en memoria dentro de cada instancia/proceso del componente, o se requiere un backend externo/compartido entre procesos/nodos?

Fuente de tiempo y determinismo: ¿Conviene que el componente lea el reloj del sistema (std::chrono::steady_clock) internamente o que reciba la marca de tiempo (timestamp) como parámetro desde el Host para facilitar pruebas deterministas?

Limpieza de claves inactivas: ¿Cómo evitar el crecimiento ilimitado de claves antiguas (por ejemplo, miles de IPs únicas) mediante expiración/TTL o límite máximo de entradas?

Granularidad del bloqueo: ¿Conviene un único std::mutex para todo el mapa de claves o particionar el bloqueo (sharding) para reducir contención bajo alta concurrencia?