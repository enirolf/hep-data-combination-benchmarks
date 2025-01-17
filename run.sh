#!/usr/bin/env bash
set -euo pipefail

N_REPS=10
SCENARIOS="scenario1_without_combinations scenario1_union_first \
           scenario1_join_first scenario2_lower_bound scenario2_upper_bound \
           scenario2_union_first scenario2_join_first"

for N_EVENTS in {1M,10M,50M,100M}; do
  printf "== ${N_EVENTS} EVENTS\n"

  for SCENARIO in $SCENARIOS; do
    printf "** ${SCENARIO}..."

    RESULTS_FILE=./results/${SCENARIO}/${N_EVENTS}.csv

    mkdir -p ${RESULTS_FILE%/*}
    touch $RESULTS_FILE
    echo "user,system,wall" > $RESULTS_FILE

    for REP in $(seq 1 $N_REPS); do
      ./clear_page_cache
      ./${SCENARIO} $N_EVENTS >> $RESULTS_FILE
    done

    printf " DONE!\n"
  done
done
