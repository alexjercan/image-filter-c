# Image Filter in C

Simple program that applies a kernel onto an image.

## Quickstart

```console
gcc main.c -o main -lm
wget https://upload.wikimedia.org/wikipedia/commons/5/50/Vd-Orig.png -O input.png
./main -i input.png -o output.pbm -f blur --cuda -r 32
```

## Features

* single thread
* multi thread - You can specify the number of threads to use with the `-p`
  flag
* cuda support - You can use cuda by using the `-c`/`--cuda` flag
* apply a filter multiple times - You can use the `-r` flag to set the number
  of repeats
* different filters - You can use the `-f` flag to set the filters
