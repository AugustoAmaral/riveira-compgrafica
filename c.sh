#!/bin/bash

compile_and_run(){
    if [[ -f $1 ]]; then
        rm $1
    fi
    make $1
    ./$1
}

if [ -z "$1" ]; then
    if [[ -f "./main.c" ]]; then
        compile_and_run "main"
    else
        echo "Digite o nome do programa sem a extensão (ou defina o nome do arquivo como main.c)"
    fi
else
    compile_and_run $1
fi
