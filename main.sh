export CPATH=/opt/homebrew/include
export LIBRARY_PATH=/opt/homebrew/lib
gcc wall.c formulas.c robot.c main.c -o main -lSDL2
./main