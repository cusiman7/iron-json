#!/bin/zsh

mkdir -p large_data

function get () {
    if [ ! -f $2 ]; then
        curl $1 > $2
    fi
}

get https://data.sfgov.org/api/views/acdm-wktn/rows.json large_data/san_fran_parcels.json
