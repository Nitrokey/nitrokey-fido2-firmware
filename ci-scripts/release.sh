#!/bin/bash
set -euo pipefail
# defeine masked variable $GITHUB_TOKEN in gitlab webinterface: Reposetorry -> Settings -> CI/CD ->Variables
# GITHUB_TOKEN can be crataed at https://github.com/settings/tokens (from machine user account)
# set privileges to full repo
# USAGE create annotated tag e.g.: v0.4.2.nitrokey (changing the name format will break things)
# run ./releas.sh

# Define Variables
repo=Nitrokey/$(basename `git rev-parse --show-toplevel`)
release_name=$(git describe)
file_version=${release_name%.*}
file_version=${file_version#v}
echo release_name:
echo $release_name
echo file_version:
echo $file_version

# Creating Release
upload_url=$(curl -s -H "Authorization: token $GITHUB_TOKEN" -d '{"tag_name": "'"$release_name"'", "name":"'"$release_name"'","body":""}' "https://api.github.com/repos/$repo/releases" | jq -r '.upload_url')
upload_url="${upload_url%\{*}"

# Collecting asset
zip -r builds.zip builds

# Uploading asset
echo "uploading asset to release to url : $upload_url"
curl -s -H "Authorization: token $GITHUB_TOKEN"  \
        -H "Content-Type: application/zip" \
        --data-binary @builds.zip  \
        "$upload_url?name=release.zip"
