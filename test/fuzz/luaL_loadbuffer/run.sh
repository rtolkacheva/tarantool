#!/bin/bash

log=log.txt
detailed_log=detailed_log.txt
runs=10000
digits=4
protos=./protos

compile() {
    cmake --build build -t luaL_loadbuffer_fuzzer
}

generate() {
    rm -rf $protos
    mkdir $protos
    export LPM_DUMP_PROTO_INPUT="1"
    export LUA_FUZZER_PADDING=$digits
    export LUA_FUZZER_PROTOS=$protos
    ./build/test/fuzz/luaL_loadbuffer/luaL_loadbuffer_fuzzer -runs=$runs
    echo files generated: `ls $protos | wc -l`
}

check() {
    error_count=0
    snap_restore_count=0
    trace_abort_count=0
    trace_record_count=0

    for file in $protos/*.test
    do
        echo "Processing $file"
        output=`./build/test/fuzz/luaL_loadbuffer/luaL_loadbuffer_fuzzer $file 2>&1 | tail --lines=4`
        line_number=1
        while IFS= read -r line; do
            if [ `echo $line | awk '{print $(NF-1)}'` -gt 0 ]; then
                case "$line_number" in
                    1) ((error_count++));;
                    2) ((snap_restore_count++));;
                    3) ((trace_abort_count++));;
                    4) ((trace_record_count++));;
                esac
            fi
            ((line_number++))
        done <<< "$output"
    done

    echo "Total number of samples: `ls $protos/*.test | wc -l`"
    echo "Total number of samples with errors: $error_count"
    echo "Total number of samples with snap restores: $snap_restore_count"
    echo "Total number of samples with abort traces: $trace_abort_count"
    echo "Total number of samples with recorded traces: $trace_record_count"
}

rename() {
    cd $protos
    for file in [0-9]*.test; do
        # Extract the numeric part of the filename
        num=${file%%.*}
        # zeroth file will be 
        num=$(echo "$num" | grep -o '0$\|[1-9][0-9]*' | tail -n 1)

        # Pad the numeric part with leading zeros
        padded_num=$(printf "%0${digits}d" "$num")

        
        # Extract the extension of the file
        extension="${file#*.}"
        
        # Construct the new filename
        new_filename="${padded_num}.${extension}"

        # Rename the file
        if [ "$file" != "$new_filename" ]; then
            mv "$file" "$new_filename"
        fi
    done
    cd -
}

check_verbose() {
    export LUA_FUZZER_VERBOSE=1
    echo -n > $log
    echo -n > $detailed_log

    num_threads=10
    for ((threadnum = 0; threadnum < num_threads; threadnum++)); do
        (
            log_t=${log}_${threadnum}
            detailed_log_t=${detailed_log}_${threadnum}

            echo -n > $log_t
            echo -n > $detailed_log_t
            for file in $protos/49*$threadnum.test
            do
                echo "Processing $file"
                output=`./build/test/fuzz/luaL_loadbuffer/luaL_loadbuffer_fuzzer $file 2>&1 | grep -m 1 -E '^luaL_loadbuffer|^lua_pcall'`

                while IFS= read -r line; do
                    case "$line" in
                        "") ;;
                        *) echo ${file##*/}: $line >> $detailed_log_t ;;
                    esac

                    # take only interesting information 
                    # line=`echo $line | awk -F ': ' '{print $NF}' | sed 's/([^)]*)//g'`
                    
                    # case "$line" in
                    #     "") ;;
                    #     *\'*\'*\'*\'*) echo $line >> $log_t ;;
                    #     *\'*\'*) echo $line | awk -F "'" '{print $(NF-2) "<name>" $NF}' >> $log_t ;;
                    #     *) echo $line >> $log_t ;;
                    # esac
                done <<< "$output"
            done
        ) &
    done

    wait

    cat ${log}_* > ${log}
    rm -f ${log}_*
    cat ${detailed_log}_* | sort > ${detailed_log}
    rm -f ${detailed_log}_*
}

process_verbose() {
    # sort $log | uniq -c | sort -nr
    # echo total error count: `cat $log | wc  -l`
    awk -f ../errors.awk $detailed_log | sort -nr
}

hide_errorless_files() {
    tmp_extension=tmp
    for file in $protos/*.test
    do
        mv $file $file.$tmp_extension
    done

    while IFS= read -r line; do
        file=${line%%:*}
        file=$protos/${file##*/}

        if [ -e $file.$tmp_extension ]; then
            mv $file.$tmp_extension $file
        fi
        # Process each line here
    done < "$detailed_log"
    # rm -f $protos/*.$tmp_extension

    echo "Errorous file count: `ls $protos/*.test | wc -l`"
}

regenerate() {
    compile
    generate
}

verbose() {
    check_verbose
    process_verbose
}

while [[ "$#" -gt 0 ]]; do
    case $1 in
        regenerate) SUBCOMMAND=regenerate ;;
        verbose) SUBCOMMAND=verbose ;;
        compile) SUBCOMMAND=compile ;;
        generate) SUBCOMMAND=generate ;;
        check) SUBCOMMAND=check ;;
        rename) SUBCOMMAND=rename ;;
        check_verbose) SUBCOMMAND=check_verbose ;;
        process_verbose) SUBCOMMAND=process_verbose ;;
        hide_errorless_files) SUBCOMMAND=hide_errorless_files ;;
        log=*) log="${1#*=}" ;;
        dlog=*) detailed_log="${1#*=}" ;;
        runs=*) runs="${1#*=}" ;;
        digits=*) digits="${1#*=}" ;;
        protos=*) protos="${1#*=}" ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

echo "Running ${SUBCOMMAND}"
$SUBCOMMAND