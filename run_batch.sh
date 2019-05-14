#!/bin/bash

for LOSS_RATE in 0.0 0.05 0.2 0.50; do
    ./run.sh $LOSS_RATE
    rm results_${LOSS_RATE}/*
    mkdir -p results_${LOSS_RATE}
    mv results/* results_${LOSS_RATE}/
done