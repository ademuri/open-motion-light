#!/bin/bash

PORT="/dev/ttyUSB0"
BAUD=115200

stty -F $PORT $BAUD raw -echo -hupcl

while true; do
  echo "Hello $(date +%s)" > $PORT
done
