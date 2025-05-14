#!/bin/bash

# Uygulamanın bulunduğu dizin
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Oyunu başlat
"$DIR/KungligaProject" client localhost
