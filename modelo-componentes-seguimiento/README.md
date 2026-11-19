1. Frontera del ABI Segura (C-API y noexcept)
Qué resuelve en el modelo: Evita el name mangling de C++ y diferencias de ABI entre compiladores o versiones al cruzar el límite entre el Host y la biblioteca dinámica (.dll / .so / .dylib).

En el código (IComponent.hpp y GreeterComponent.cpp):

Las funciones de fábrica y ciclo de vida se exponen con extern "C" (getApiVersion, createComponent, destroyComponent) y se tipan como punteros a función (GetApiVersionFunc, CreateComponentFunc, DestroyComponentFunc).

Tanto las funciones extern "C" como el destructor virtual de IComponent (virtual ~IComponent() noexcept = default) y los métodos de interfaz llevan noexcept.

Además, se valida CURRENT_API_VERSION antes de instanciar para prevenir lecturas inválidas de la vtable si cambian los métodos de la interfaz.

2. Eliminación de Excepciones Cruzadas y Tipos C++ Estándar
Qué resuelve en el modelo: Lanzar excepciones C++ a través del límite de una biblioteca compartida o pasar tipos de la STL (std::string, std::vector) por la frontera binaria es inseguro por diferencias de layout o runtime.

En el código (IComponent.hpp, IGreeter.hpp, GreeterComponent.cpp):

En la frontera pública se usan tipos primitivos / estilo C (const char*, char* outBuffer, size_t bufferSize) y un enum class ComponentResult : int (SUCCESS, ERROR_INVALID_ARGUMENT, ERROR_INTERNAL) en lugar de devolver objetos C++ o propagar excepciones.

Internamente GreeterComponent puede usar std::string (prefix), pero en greet y createComponent envuelve la lógica en try / catch (...) para absorber cualquier excepción y devolver un código de error o nullptr sin que escape hacia el Host.

3. Gestión Automática de Memoria Cruzada (RAII con Custom Deleters)
Qué resuelve en el modelo: Aplica el principio "el que reserva la memoria, la libera" y evita tanto la corrupción del heap como que se descargue la biblioteca (dlclose / FreeLibrary) mientras existan instancias vivas.

En el código (SharedLibrary.hpp y ModuleManager.hpp):

SharedLibrary encapsula el handle nativo (LoadLibrary / dlopen y FreeLibrary / dlclose) bajo RAII y deshabilita copia (= delete) para no cerrar el handle por accidente.

ModuleManager::createInstance crea el componente vía createFunc() (en el heap del módulo) y devuelve un std::shared_ptr<InterfaceType> con un custom deleter [destroyFunc, lib](InterfaceType* ptr) { destroyFunc(ptr); }.

Al capturar lib (std::shared_ptr<SharedLibrary>) por valor en la lambda, el handle de la biblioteca se mantiene vivo mientras siga existiendo al menos una instancia del componente, y al liberarse el puntero se invoca destroyFunc en el contexto del propio módulo.

4. Seguridad de Hilos y Concurrencia Primitiva
Qué resuelve en el modelo: Evita data races o corrupción del registro de bibliotecas cargadas si varios hilos del Host consultan o cargan módulos de forma concurrente.

En el código (ModuleManager.hpp):

ModuleManager protege loadedLibraries (std::unordered_map<std::string, std::shared_ptr<SharedLibrary>>) con un std::mutex mapMutex y bloqueos RAII mediante std::lock_guard<std::mutex> en loadModule y createInstance (acotando el ámbito del lock para minimizar el tiempo bloqueado).

5. Rechazo a std::exit en Infraestructura (Propagación vía Excepciones)
Qué resuelve en el modelo: Llamar a std::exit dentro de la infraestructura corta el programa de golpe sin hacer stack unwinding ni ejecutar destructores locales, dejando recursos abiertos.

En el código (SharedLibrary.hpp y ModuleManager.hpp):

Ante fallos de carga (!handle), módulo no precargado, símbolos faltantes (getApiVersion, createComponent, destroyComponent), discrepancia de ABI o fallo de dynamic_cast, se lanza std::runtime_error en lugar de llamar a std::exit.

Así el error sube limpiamente hasta el Host (main / Application), permitiendo desenredar la pila y liberar recursos RAII de forma ordenada.