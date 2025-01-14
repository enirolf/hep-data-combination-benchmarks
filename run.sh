#!/usr/bin/env bash
set -euo pipefail

N_REPS=5

for N_EVENTS in {1M,10M,50M,100M}; do
  printf "${N_EVENTS} EVENTS\n"

  for SCENARIO in {1..3}; do
    printf "** SCENARIO ${SCENARIO}..."

    RESULTS_FILE=./results/scenario${SCENARIO}/${N_EVENTS}.csv

    mkdir -p ${RESULTS_FILE%/*}
    touch $RESULTS_FILE
    echo "user,system,wall" > $RESULTS_FILE

    for REP in $(seq 1 $N_REPS); do
      ./clear_page_cache
      ./scenario${SCENARIO} $N_EVENTS >> $RESULTS_FILE
    done

    printf " DONE!\n"
  done
done
