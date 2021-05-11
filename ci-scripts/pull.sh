#!/bin/bash
set -euo pipefail
# repo_name e.g.= pynitrokey
repo_name="$1"

for branch in $(git for-each-ref --format='%(refname)' refs/heads/); do
	if [ "master" = "${branch#*/*/}" ]; then
	    echo "no force"
	    echo "${branch#*/*/}"
	else
	    echo "force"
	    echo "${branch#*/*/}"
	fi
done

mkdir git-magic
cd git-magic
git clone --mirror https://github.com/Nitrokey/$repo_name.git
cd $repo_name.git
git remote add --mirror=fetch secondary https://git.dotplex.com/Nitrokey/$repo_name.git
git remote set-url secondary https://oauth2:$GITLAB_REPO_KEY@git.dotplex.com/Nitrokey/$repo_name.git
git fetch origin
git push secondary --tags

for branch in $(git for-each-ref --format='%(refname)' refs/heads/); do
	if [ "master" = "${branch#*/*/}" ]; then
	    echo "${branch#*/*/}"
	    git push secondary ${branch#*/*/}
	else
	    echo "${branch#*/*/}"
	    if ! git push secondary ${branch#*/*/}; then
	      	    git push secondary ${branch#*/*/} --force
	    fi
	fi
done
