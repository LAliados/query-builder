Makefile
BUILD_DIRECTORY = build
DEBUG_BUILD_DIRECTORY = build_debug
LIBRARY_DIRECTORY = $(CURDIR)/lib


all: $(BUILD_DIRECTORY)
	cmake -B $(BUILD_DIRECTORY)  
	cmake --build $(BUILD_DIRECTORY) -- -j$(nproc) 
	mkdir -p lib 
	cmake --install $(BUILD_DIRECTORY) --prefix $(LIBRARY_DIRECTORY) 
debug: $(BUILD_DIRECTORY)
	cmake -B $(DEBUG_BUILD_DIRECTORY) -DCMAKE_BUILD_TYPE=Debug 
	cmake --build $(DEBUG_BUILD_DIRECTORY) -- -j$(nproc) 
	mkdir -p lib 
	cmake --install $(DEBUG_BUILD_DIRECTORY) --prefix $(LIBRARY_DIRECTORY) 

$(BUILD_DIRECTORY):
	mkdir -p $(BUILD_DIRECTORY)

clean:
	rm -rf $(BUILD_DIRECTORY) $(DEBUG_BUILD_DIRECTORY) $(LIBRARY_DIRECTORY)