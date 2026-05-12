Это рассчитано на сборку в Windows через MSYS2 (целиком, включая компилятор). Успешно собирается +- всё, кроме OPC UA через 62541.

Чтобы Visual Studio Code корректно считывал изменения при вызовах CMake извне, в папке проекта нужно добавить .vscode/c_cpp_properties.json:
```json
{
    "configurations": [
    {
        "name": "CMake",
        "compileCommands": "${config:cmake.buildDirectory}/compile_commands.json",
        "configurationProvider": "ms-vscode.cmake-tools"
    }
    ],
    "version": 4
}
```