# Aurora
A toy OpenGL Render Engine.

## Platform support
| Platform | CI |
|----------|:--:|
| Windows | <p>[![MSVC CI](https://github.com/JHex-git/Aurora/actions/workflows/MSVC.yml/badge.svg)](https://github.com/JHex-git/Aurora/actions/workflows/MSVC.yml)</p> <p>[![MinGW CI](https://github.com/JHex-git/Aurora/actions/workflows/MinGW.yml/badge.svg)](https://github.com/JHex-git/Aurora/actions/workflows/MSVC.yml)</p> |

## Feature
* Phong shading
* Skybox
* Outline
* Shadow mapping
* Bounding Volume Hierarchy

## Snapshots
![Overview](docs/snapshots/overview.png)

## How to build
Only MinGW and MSVC are tested. For MinGW, MSYS2 is recommended as the version of its g++ has a good support for C++23. Run `MinGW_build.bat` or `MSVC_build.bat` to build the project.

## Credits
Credits for models used in this project can be found in [CREDITS](./CREDITS)