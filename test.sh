#!/bin/bash

# Test script for the iBug Shell (ish).
# It should execute properly when sent via stdin

# Comments should be ignored
echo This is a sentence. # Comment, blah blah blah~~~
pwd # asdfghjkl
echo "This is a sentence. # Not comment" # Comment

# exec-ing external commands
ls -A
touch sh
which touch

# Built-in commands (no-piping for builtins)
pwd
cd src
pwd
cd ..
pwd

# Piping
seq 10 | tac
seq 10 | tac | cat -n

# Required redirection
seq 10 > sh.o
cat < sh.o
seq 10 >> sh.o
< sh.o tac | grep 1 > sh2.o
rm sh.o sh2.o

# Variable setting & expansion
echo $PWD
echo "$PWD"
echo ${HOME}
echo '$HOME' # No expansion in single quotes
A=3
echo "$A"
echo "$A.suffix"
echo "$AAA" # Should not echo anything
echo "${A}AA"
env | grep 'A='
export A=3
env | grep 'A='

# Extended redirection
cat <<< "Oh, haha, right?"
cat <<< "Home: $HOME"
cat << haha
This is a multi-line heredoc
It also \$upports e\$caping\!
Your \$HOME is $HOME
haha
