#!/bin/bash

ARGS=$@
echo
echo "Running tests"
echo 
RED='\033[0;31m'
NO_COLOR='\033[0m'
GREEN='\033[0;32m'


PASSED=0
FAILED=0
TOTAL=0

for ARG in ${ARGS}
do
  let TOTAL++
  ${ARG}
  STATUS=$?
  if [[ ${STATUS} -gt 0 ]]; then
    echo -e " \t Test ${ARG} ${RED}FAILED${NO_COLOR}"
    echo
    break
  else
    echo -e " \t Test ${ARG} ${GREEN}PASSED${NO_COLOR}"
  fi
  echo
done

SUCCESS=""
if [[ ${STATUS} -gt 0 ]]; then
  SUCCESS="$(echo -e ${RED}FAILURE${NO_COLOR})"
else

  SUCCESS="$(echo -e ${GREEN}SUCCESS${NO_COLOR})"
fi

echo "  -------------------------------------------------------"
echo "  | Results: ${SUCCESS} (${TOTAL} tests, ${PASSED} successes, ${FAILED} failures) |"
echo "  -------------------------------------------------------"
echo 
echo
