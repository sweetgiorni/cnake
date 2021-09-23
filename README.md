# cnake

Python bindings for CMake.

<br/>

### How to build on Ubuntu
Install build tools:

`sudo apt install -y cmake clang-12 ninja-build ccache`

Install dependencies:

`sudo apt install -y libboost-all-dev python3.8-dev`

Build:

`mkdir build && cd build && cmake -G Ninja .. && ninja`