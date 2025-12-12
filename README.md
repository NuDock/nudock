# NuDock

## Building & installation

First get the nudock library:

```bash
git clone --recurse-submodules git@github.com:ArturSztuc/nudock.git
cd nudock
```

To build & install locally:

```bash
# Configure makefiles
cmake -B build -DCMAKE_INSTALL_PREFIX:PATH=/path/to/some/local/install/folder/
# Build NuDock
cmake --build build -j9
# Installs NuDock in /path/to/some/local/install/folder
cmake --install build
```

To build the `test_server` and `test_client` tests:

```bash
cd tests
# Configure the makefiles.
cmake -B build -DCMAKE_PREFIX_PATH=/path/to/NuDock/install/folder

# Builds the tests
cmake --build build
```

You can now enter the build directory and first run `./test_server`, and then, in a separate terminal, run `./test_client`. If everything goes well, the client should be able to communicate with the server by sending validating versions against each other first and then setting osc/syst parameters & asking for log_likelihoods.
