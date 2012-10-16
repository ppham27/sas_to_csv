#!/bin/env sh

rm ./test_files/*.csv
rm ./test_files/*.info
for f in ./test_files/*.sas7bdat ; do
    echo $f
    ./bin/sas_to_csv $f
done
