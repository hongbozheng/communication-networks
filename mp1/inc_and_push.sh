#!/bin/bash
typeset -i version=$(cat version.txt)

printf "\n[INFO]: Changing version from $version to $((version+1))\n\n"
version=$((version+1))
echo "$version" > version.txt

git add -u
git commit -m ":bookmark: Increment version to $version :bookmark:"
git push origin main