#!/bin/bash

export OPTIMIZATION=0
make test_all
export OPTIMIZATION=1
make test_all
export OPTIMIZATION=2
make test_all
