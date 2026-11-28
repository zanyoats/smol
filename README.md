# smol

## Build the library

Requires CMake 3.25+, Ninja, and a C++20 compiler.
The library is header-only; this builds its examples and tests.

```sh
cmake --preset debug
cmake --build --preset debug --parallel
```

## Run the examples

After building:

```sh
./build/debug/examples/greeting/greeting
```

> [!NOTE]
> In VS Code on macOS, open `examples/<name>/main.cpp` and press **Cmd+Shift+B**
> to build and run that example.

## Run the tests

After building:

```sh
ctest --preset debug
```

> [!NOTE]
> In VS Code, select **Tasks: Run Task → smol: Run tests** from the Command Palette.
> This builds the tests before running them.
>
> For **Cmd+Shift+T** on macOS, add this entry to the array in
> **Preferences: Open Keyboard Shortcuts (JSON)**:
>
> ```json
> {
>   "key": "cmd+shift+t",
>   "command": "workbench.action.tasks.runTask",
>   "args": "smol: Run tests",
>   "when": "config.smol.enableTestShortcut == true"
> }
> ```
>
> The project enables this shortcut through `.vscode/settings.json`. It replaces
> the reopen-closed-editor shortcut only in this workspace; other projects keep
> their normal binding. VS Code may label the custom setting as unknown, but the
> keybinding condition can still read it.
