# query-builder  (In developing)
Creates SQL query string (only PostgreSQL supported now)

## Building
To build and install into ./lib/ 
``` bash
make
```
Or to build and install into your dir:
``` bash
mkdir -p build
cmake -B build  
cmake --build build  
cmake --install build --prefix <your dir> 
```

## Building tests
``` bash
cd tests && make
```

