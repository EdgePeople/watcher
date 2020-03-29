#!/bin/bash
ls ./ | grep 'run\.out' -v | grep 'main\.cpp' -v | grep 'build\.sh' -v | grep 'clean\.sh' -v | grep 'test\.sh' -v | xargs rm -rf --