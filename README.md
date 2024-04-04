# Rufus

kernel.bin exporter/importer in command line

## Build

Requires Qt 6.

## Examples

~~~sh
# Show help
./rufus -h
# Export texts to CSV
./rufus -f "scene.csv" "scene.bin"
# Import texts from CSV
./rufus -i -f "scene.csv" -k "kernel.bin" "scene.bin"
~~~

You can also use `-j` for the Japanese format.

**Note about the kernel argument:** When modifying the scene.bin, some
data size may be modified, and a list of pointers inside the kernel.bin
must be updated to prevent any crashes of the game.
Passing the `-k` argument is optional but recommended.
