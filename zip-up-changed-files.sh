#!/bin/bash
SELF="$(readlink -f "${BASH_SOURCE[0]}")"
cd "${SELF%/*}"
set -ex

COMMITS=( $(git log net-next/master..master --pretty=format:%H) )
declare -A FILES
for commit in "${COMMITS[@]}"; do
	for file in $(git diff-tree --no-commit-id --name-only -r "$commit"); do
		if [[ -n ${FILES["$file"]} ]]; then
			FILES["$file"]="_"
		else
			FILES["$file"]="$commit"
		fi
	done
done

while read -r status file; do
	[[ $status == M || $status == D || $status == A ]] || continue
	commit="${FILES["$file"]}"
	[[ -n $commit && $commit != _ ]] || continue
	git commit --fixup="$commit" "$file"
done < <(git status --porcelain -uno)

GIT_SEQUENCE_EDITOR=true git rebase -i --autosquash net-next/master
