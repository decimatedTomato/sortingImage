#!/bin/bash
set -xe 
gcc src/* -o bin/imageSort -I includes -Wall -Wextra
# gcc src/* -o bin/imageSort -I includes -Wall -Wextra -lm -fsanitize=address