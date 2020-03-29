#!/bin/bash

counter=0
while [ $counter -lt 10 ]; do
    counter=`expr $counter + 1`
    echo "test content $counter\nasdasdlkasnlk asjnjaksnk adjsandna aisdnasndkjasnd" > waa$counter
done