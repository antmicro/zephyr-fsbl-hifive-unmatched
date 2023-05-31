#!/bin/bash
EXPECT=$@
MISSING=

echo "Expecting artifacts: $EXPECT"

for f in $EXPECT; do
  test -f $f || MISSING+=" $f"
done

if [ -z "$MISSING" ]; then
  echo "All artifacts present."
  exit 0
else
  echo "Missing artifacts: $MISSING"
  exit 1
fi
