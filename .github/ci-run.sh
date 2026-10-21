#!/bin/bash
# usage: ci-run.sh <name> <command...>
# Runs a command, keeps its log in /tmp/<name>.log and, on failure, publishes the log tail as an annotation.
name=$1; shift
log=/tmp/$name.log
"$@" 2>&1 | tee "$log"
rc=${PIPESTATUS[0]}
if [ "$rc" -ne 0 ]; then
  m=$(tail -n 60 "$log" | sed ':a;N;$!ba;s/\n/%0A/g')
  echo "::error title=$name failed (rc=$rc)::$m"
fi
exit "$rc"
