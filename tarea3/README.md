
# Comandos Disponibles

1. **help**:
   - **Descripción**: Muestra un mensaje de ayuda con la lista de comandos disponibles y sus descripciones.
   - **Sintaxis**: `help`

2. **exit**:
   - **Descripción**: Sale del programa.
   - **Sintaxis**: `exit`

3. **touch <filename>**:
   - **Descripción**: Crea un nuevo archivo vacío con el nombre especificado.
   - **Sintaxis**: `touch <filename>`
   - **Ejemplo**: `touch archivo.txt`

4. **mkdir <dirname>**:
   - **Descripción**: Crea un nuevo directorio con el nombre especificado.
   - **Sintaxis**: `mkdir <dirname>`
   - **Ejemplo**: `mkdir carpeta`

5. **rename <oldname> <newname>**:
   - **Descripción**: Renombra un archivo o directorio existente.
   - **Sintaxis**: `rename <oldname> <newname>`
   - **Ejemplo**: `rename archivo.txt nuevo_nombre.txt`

6. **rm <name>**:
   - **Descripción**: Elimina un archivo o directorio. Si el directorio no está vacío, también elimina todo su contenido recursivamente.
   - **Sintaxis**: `rm <name>`
   - **Ejemplo**: `rm archivo.txt` o `rm carpeta`

7. **chmod <path> <mode>**:
   - **Descripción**: Cambia los permisos de un archivo o directorio.
   - **Sintaxis**: `chmod <path> <mode>`
   - **Ejemplo**: `chmod archivo.txt 755`

8. **ls**:
   - **Descripción**: Lista los archivos en el directorio actual.
   - **Sintaxis**: `ls`

9. **ls -R**:
   - **Descripción**: Lista los archivos en el directorio actual y en sus subdirectorios de manera recursiva, mostrando información detallada (número de inodo, permisos, número de enlaces, propietario, grupo, tamaño, fecha y hora de modificación, nombre de archivo).
   - **Sintaxis**: `ls -R`

10. **ls -l**:
    - **Descripción**: Lista los archivos en el directorio actual mostrando información detallada (número de inodo, permisos, número de enlaces, propietario, grupo, tamaño, fecha y hora de modificación, nombre de archivo).
    - **Sintaxis**: `ls -l`

11. **cd <path>**:
    - **Descripción**: Cambia el directorio actual al especificado.
    - **Sintaxis**: `cd <path>`
    - **Ejemplo**: `cd carpeta`

12. **pwd**:
    - **Descripción**: Imprime el directorio de trabajo actual.
    - **Sintaxis**: `pwd`

13. **search <name>**:
    - **Descripción**: Busca archivos y directorios por nombre en el directorio actual y sus subdirectorios.
    - **Sintaxis**: `search <name>`
    - **Ejemplo**: `search archivo`

14. **history**:
    - **Descripción**: Muestra el historial de comandos ejecutados.
    - **Sintaxis**: `history`

### Nota
- Todos los comandos deben ser escritos en minúsculas y respetar los espacios y la sintaxis proporcionada.
- Los nombres de archivos y directorios no deben contener caracteres especiales.
