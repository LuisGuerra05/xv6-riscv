# INFORME – Tarea 0: Instalación y Ejecución de xv6

**Nombre: Luis Guerra**

## 1. Pasos Seguidos para Instalar xv6

### 1.1 Preparación del entorno
Primero fue necesario contar con un sistema Linux para poder compilar y ejecutar xv6.  
Las dos alternativas posibles eran:  
- **Opción 1 (ideal):** usar **Ubuntu a través de WSL** en Windows.  
- **Opción 2 (plan B):** usar **Ubuntu en una máquina virtual** (VirtualBox/VMware).  

En este caso se eligió la primera opción, instalando **Ubuntu 24.04.1 LTS de forma local en WSL**.


### 1.2 Creación del fork en GitHub
1. Abrir el repositorio original: [mit-pdos/xv6-riscv](https://github.com/mit-pdos/xv6-riscv).  
2. Seleccionar la opción **Fork** para crear una copia en la cuenta personal de GitHub.  
3. (Opcional) Activar la opción *“Copy the riscv branch only”* según lo indicado por el profesor.  
4. El fork queda disponible en la dirección: https://github.com/USUARIO/xv6-riscv

    En este caso particular:  https://github.com/LuisGuerra05/xv6-riscv


### 1.3 Configuración de Git
Antes de poder configurar Git, es necesario tenerlo instalado en el sistema.  
Esto se realiza con el siguiente comando:

```bash
sudo apt install -y git
```

Una vez instalado, en la terminal de Ubuntu se deben registrar los datos de usuario en Git:

```bash
git config --global user.name "Nombre Apellido"
git config --global user.email "correo@ejemplo.com"
```
En este caso particular, la configuración fue:

```bash
git config --global user.name "Luis Guerra"
git config --global user.email "luguerra@alumnos.uai.cl"
```

Además, para autenticar con GitHub, se recomienda generar un Personal Access Token (PAT):

- Ir a GitHub → Settings → Developer settings → Personal access tokens → Tokens (classic) → Generate new token.

- Seleccionar permisos sobre repo.

- Guardar el token en un lugar seguro.

Configurar Git para recordar las credenciales:

```bash
git config --global credential.helper store
```

La primera vez que se realice un git push, Git solicitará usuario y contraseña. En el campo de contraseña se debe ingresar el PAT.

### 1.4 Clonación del fork

Clonar el repositorio forkeado desde la cuenta personal de GitHub:
```bash
git clone https://github.com/USUARIO/xv6-riscv.git
cd xv6-riscv
```

### 1.5 Creación de rama de trabajo

En este curso se trabajará utilizando **una rama diferente para cada tarea**.  
Esto permite mantener el código del repositorio organizado y separar claramente los avances de cada entrega.

Para crear una nueva rama se utiliza el comando:

```bash
git checkout -b nombre_apellido_tx
```

Donde:
- nombre_apellido corresponde a los datos del estudiante.
- tx corresponde al número de la tarea (t0, t1, t2, etc.).

### 1.6 Instalación de dependencias en Ubuntu

Actualizar los paquetes e instalar las herramientas necesarias para compilar y ejecutar xv6:
```bash
sudo apt update
sudo apt install -y make qemu-system-misc bc gcc-riscv64-unknown-elf
```
- `make` → para compilar el proyecto con el Makefile.
- `qemu-system-misc` → incluye qemu-system-riscv64, el emulador donde se ejecuta xv6.
- `bc` → calculadora en línea de comandos que xv6 usa en algunos scripts de compilación.
- `gcc-riscv64-unknown-elf` → compilador cruzado para compilar xv6 a RISC-V.

### 1.7 Compilación y ejecución de xv6

Compilar y ejecutar xv6 desde la carpeta raíz del repositorio:

```bash
make qemu
```
Para salir de QEMU:
```css
Ctrl + A   →   X
```


## 2. Problemas Encontrados y Soluciones

Durante la instalación y ejecución de xv6 se presentaron algunos problemas, los cuales se describen a continuación junto con sus soluciones.

### 2.1 Error al instalar WSL: `WslRegisterDistribution failed with error: 0x80070422`
**Descripción:**  
Al intentar instalar Ubuntu desde Microsoft Store apareció el error:
```bash
WslRegisterDistribution failed with error: 0x80070422
The service cannot be started, either because it is disabled or because it has no enabled devices associated with it.
```

**Causa probable:**  
El servicio de WSL o algunos servicios relacionados (Windows Update, BITS, LxssManager) estaban deshabilitados.

**Solución aplicada:**  
De acuerdo a una referencia encontrada en [Reddit](https://www.reddit.com/r/bashonubuntuonwindows/comments/1by8mb3/wslregisterdistribution_failed_with_error/), la solución consistió en:
- Configurar el **Servicio WSL** en modo *Manual* desde la herramienta **Servicios** de Windows.  
- Verificar que no hubiera programas como *CCleaner* u otros que bloquearan la ejecución de “Subsistema de Windows para Linux Update” o “Windows Update Health Tools”.  

Con este ajuste, la instalación de Ubuntu pudo completarse correctamente.


### 2.2 Problemas con autenticación en GitHub

**Descripción:**
Al ejecutar git push se solicitaba usuario y contraseña, pero GitHub ya no acepta contraseñas normales.

**Solución:**
Generar un Personal Access Token (PAT) en GitHub y utilizarlo como contraseña en el primer git push.
Para no ingresar el token cada vez, como ya se mencionó en la seccion anterior, se configuró Git con:

```bash
git config --global credential.helper store
```

**NOTA:** Otros errores comunes durante la instalación suelen estar relacionados con la falta de QEMU o del compilador de RISC-V. En este caso no se presentaron dichos problemas, ya que se instaló todo lo necesario desde el inicio.

## 3. Confirmación de que xv6 está Funcionando Correctamente

Para comprobar que la instalación fue exitosa, se ejecutaron dentro de xv6 los siguientes comandos:

```bash
ls
echo "Hola xv6"
cat README
```

Los resultados obtenidos fueron los esperados:
- `ls` mostró la lista de archivos del sistema.
- `echo "Hola xv6"` imprimió correctamente el texto.
- `cat README` desplegó el contenido del archivo README.

Además, se adjunta la siguiente captura de pantalla como evidencia: