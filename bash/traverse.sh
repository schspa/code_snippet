#!/bin/bash

function traverse()
{
	local dir=${1}
	for file in $(ls ${dir})
	do
		current=${dir}"/"${file}
		if [[ -d ${current} ]]; then
			traverse ${current}
		else
			echo ${current}
		fi
	done
}

if [[ $# -lt 1 ]]; then
	traverse .
else
	traverse $1
fi
