BUILD_DIR = build
OUTPUT_DIR = $(BUILD_DIR)/output
INSTALL_DIR = $(BUILD_DIR)/install


.PHONY: clean build rebuild sub-update

build:
	@echo "Starting build process... $(shell nproc) cores"
	cmake -B $(BUILD_DIR) -DNETWORK_ARMORY_BUILD_TESTS=ON -DNETWORK_ARMORY_BUILD_EXAMPLE=ON
	cmake --build $(BUILD_DIR) -j$(shell nproc)
	# cp $(BUILD_DIR)/example/network_armory_example $(OUTPUT_DIR)


clean:
	rm -rf $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)
	rm -rf $(OUTPUT_DIR)
	mkdir -p $(OUTPUT_DIR)

rebuild: clean build

sub-update:
	@echo "Updating submodules..."
	git submodule update --init --recursive