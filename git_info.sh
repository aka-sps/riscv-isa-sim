#!/bin/bash

export OUT_FNAME="info"

export remote=`git remote -v | grep -E "origin.*fetch" | cut -d$'\t' -f2 | cut -d' ' -f1`
export branch=`git branch | grep '* ' | cut -d' ' -f2`
export commit=`git log -1 | grep "^commit " | cut -d' ' -f2`

printf "RTL:\n"                     > ${OUT_FNAME}
printf "scrx_cluster:\n"            >> ${OUT_FNAME}
printf "remote: %s\n" ${remote}     >> ${OUT_FNAME}
printf "branch: %s\n" ${branch}     >> ${OUT_FNAME}
printf "commit: %s\n" ${commit}     >> ${OUT_FNAME}

export submodules_list=`git submodule`
export path_s='xxxx'
export sha_s='xxxx'

export top_dir=${PWD}

for tmp_str in ${submodules_list} ; do
    if [ ${sha_s} = "xxxx" ] ; then 
        export sha_s=${tmp_str}
        continue;
    fi
    if [ ${path_s} = "xxxx" ] ; then 
        export path_s=${tmp_str}
        continue;
    fi

    export submodule_name=`echo ${path_s} | cut -d'/' -f2-`
    
    printf "\n%s:\n" ${submodule_name} >> ${OUT_FNAME}
    cd ${path_s}
    export remote=`git remote -v | grep -E "origin.*fetch" | cut -d$'\t' -f2 | cut -d' ' -f1`
    export branch=`git branch | grep '* ' | grep "detached" | cut -d' ' -f2-`

    if [ -z "${branch}" ] ; then
        export branch=`git branch | grep '* ' | cut -d' ' -f2`
    fi

    export commit=`git log -1 | grep "^commit " | cut -d' ' -f2`
    cd ${top_dir}
    printf "remote: %s\n" "${remote}" >> ${OUT_FNAME}
    printf "branch: %s\n" "${branch}" >> ${OUT_FNAME}
    printf "commit: %s\n" "${commit}" >> ${OUT_FNAME}

    export path_s='xxxx'
    export sha_s='xxxx'
done
