#!/bin/zsh

mkdir -p large_data

function get () {
    if [ ! -f $2 ]; then
        curl $1 > $2
    fi
}

get https://data.sfgov.org/api/views/acdm-wktn/rows.json large_data/san_fran_parcels.json
get https://raw.githubusercontent.com/serde-rs/json-benchmark/master/data/canada.json large_data/canada.json
get https://raw.githubusercontent.com/serde-rs/json-benchmark/master/data/twitter.json large_data/twitter.json
